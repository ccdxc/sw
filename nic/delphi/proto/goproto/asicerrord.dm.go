// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/tsdb/metrics"
)

type Ppappa0intbndl0Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	StateLkpSrchHitAddrRangeErr metrics.Counter

	StateLkpSrchMiss metrics.Counter

	StateLkpSrchReqNotRdy metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intbndl0Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intbndl0Metrics) Size() int {
	sz := 0

	sz += mtr.StateLkpSrchHitAddrRangeErr.Size()

	sz += mtr.StateLkpSrchMiss.Size()

	sz += mtr.StateLkpSrchReqNotRdy.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intbndl0Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.StateLkpSrchHitAddrRangeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.StateLkpSrchHitAddrRangeErr.Size()

	mtr.StateLkpSrchMiss = mtr.metrics.GetCounter(offset)
	offset += mtr.StateLkpSrchMiss.Size()

	mtr.StateLkpSrchReqNotRdy = mtr.metrics.GetCounter(offset)
	offset += mtr.StateLkpSrchReqNotRdy.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intbndl0Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "StateLkpSrchHitAddrRangeErr" {
		return offset
	}
	offset += mtr.StateLkpSrchHitAddrRangeErr.Size()

	if fldName == "StateLkpSrchMiss" {
		return offset
	}
	offset += mtr.StateLkpSrchMiss.Size()

	if fldName == "StateLkpSrchReqNotRdy" {
		return offset
	}
	offset += mtr.StateLkpSrchReqNotRdy.Size()

	return offset
}

// SetStateLkpSrchHitAddrRangeErr sets cunter in shared memory
func (mtr *Ppappa0intbndl0Metrics) SetStateLkpSrchHitAddrRangeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StateLkpSrchHitAddrRangeErr"))
	return nil
}

// SetStateLkpSrchMiss sets cunter in shared memory
func (mtr *Ppappa0intbndl0Metrics) SetStateLkpSrchMiss(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StateLkpSrchMiss"))
	return nil
}

// SetStateLkpSrchReqNotRdy sets cunter in shared memory
func (mtr *Ppappa0intbndl0Metrics) SetStateLkpSrchReqNotRdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StateLkpSrchReqNotRdy"))
	return nil
}

// Ppappa0intbndl0MetricsIterator is the iterator object
type Ppappa0intbndl0MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intbndl0MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intbndl0MetricsIterator) Next() *Ppappa0intbndl0Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intbndl0Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intbndl0MetricsIterator) Find(key uint64) (*Ppappa0intbndl0Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intbndl0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intbndl0MetricsIterator) Create(key uint64) (*Ppappa0intbndl0Metrics, error) {
	tmtr := &Ppappa0intbndl0Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intbndl0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intbndl0MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intbndl0MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intbndl0MetricsIterator returns an iterator
func NewPpappa0intbndl0MetricsIterator() (*Ppappa0intbndl0MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intbndl0Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intbndl0MetricsIterator{iter: iter}, nil
}

type Ppappa0intbndl1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	StateLkpSrchHitAddrRangeErr metrics.Counter

	StateLkpSrchMiss metrics.Counter

	StateLkpSrchReqNotRdy metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intbndl1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intbndl1Metrics) Size() int {
	sz := 0

	sz += mtr.StateLkpSrchHitAddrRangeErr.Size()

	sz += mtr.StateLkpSrchMiss.Size()

	sz += mtr.StateLkpSrchReqNotRdy.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intbndl1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.StateLkpSrchHitAddrRangeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.StateLkpSrchHitAddrRangeErr.Size()

	mtr.StateLkpSrchMiss = mtr.metrics.GetCounter(offset)
	offset += mtr.StateLkpSrchMiss.Size()

	mtr.StateLkpSrchReqNotRdy = mtr.metrics.GetCounter(offset)
	offset += mtr.StateLkpSrchReqNotRdy.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intbndl1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "StateLkpSrchHitAddrRangeErr" {
		return offset
	}
	offset += mtr.StateLkpSrchHitAddrRangeErr.Size()

	if fldName == "StateLkpSrchMiss" {
		return offset
	}
	offset += mtr.StateLkpSrchMiss.Size()

	if fldName == "StateLkpSrchReqNotRdy" {
		return offset
	}
	offset += mtr.StateLkpSrchReqNotRdy.Size()

	return offset
}

// SetStateLkpSrchHitAddrRangeErr sets cunter in shared memory
func (mtr *Ppappa0intbndl1Metrics) SetStateLkpSrchHitAddrRangeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StateLkpSrchHitAddrRangeErr"))
	return nil
}

// SetStateLkpSrchMiss sets cunter in shared memory
func (mtr *Ppappa0intbndl1Metrics) SetStateLkpSrchMiss(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StateLkpSrchMiss"))
	return nil
}

// SetStateLkpSrchReqNotRdy sets cunter in shared memory
func (mtr *Ppappa0intbndl1Metrics) SetStateLkpSrchReqNotRdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StateLkpSrchReqNotRdy"))
	return nil
}

// Ppappa0intbndl1MetricsIterator is the iterator object
type Ppappa0intbndl1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intbndl1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intbndl1MetricsIterator) Next() *Ppappa0intbndl1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intbndl1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intbndl1MetricsIterator) Find(key uint64) (*Ppappa0intbndl1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intbndl1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intbndl1MetricsIterator) Create(key uint64) (*Ppappa0intbndl1Metrics, error) {
	tmtr := &Ppappa0intbndl1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intbndl1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intbndl1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intbndl1MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intbndl1MetricsIterator returns an iterator
func NewPpappa0intbndl1MetricsIterator() (*Ppappa0intbndl1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intbndl1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intbndl1MetricsIterator{iter: iter}, nil
}

type Ppappa0inteccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PktMemUncorrectable metrics.Counter

	PktMemCorrectable metrics.Counter

	Bndl0StateLkpSramUncorrectable metrics.Counter

	Bndl0StateLkpSramCorrectable metrics.Counter

	Bndl1StateLkpSramUncorrectable metrics.Counter

	Bndl1StateLkpSramCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0inteccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0inteccMetrics) Size() int {
	sz := 0

	sz += mtr.PktMemUncorrectable.Size()

	sz += mtr.PktMemCorrectable.Size()

	sz += mtr.Bndl0StateLkpSramUncorrectable.Size()

	sz += mtr.Bndl0StateLkpSramCorrectable.Size()

	sz += mtr.Bndl1StateLkpSramUncorrectable.Size()

	sz += mtr.Bndl1StateLkpSramCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0inteccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PktMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PktMemUncorrectable.Size()

	mtr.PktMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PktMemCorrectable.Size()

	mtr.Bndl0StateLkpSramUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Bndl0StateLkpSramUncorrectable.Size()

	mtr.Bndl0StateLkpSramCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Bndl0StateLkpSramCorrectable.Size()

	mtr.Bndl1StateLkpSramUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Bndl1StateLkpSramUncorrectable.Size()

	mtr.Bndl1StateLkpSramCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Bndl1StateLkpSramCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0inteccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PktMemUncorrectable" {
		return offset
	}
	offset += mtr.PktMemUncorrectable.Size()

	if fldName == "PktMemCorrectable" {
		return offset
	}
	offset += mtr.PktMemCorrectable.Size()

	if fldName == "Bndl0StateLkpSramUncorrectable" {
		return offset
	}
	offset += mtr.Bndl0StateLkpSramUncorrectable.Size()

	if fldName == "Bndl0StateLkpSramCorrectable" {
		return offset
	}
	offset += mtr.Bndl0StateLkpSramCorrectable.Size()

	if fldName == "Bndl1StateLkpSramUncorrectable" {
		return offset
	}
	offset += mtr.Bndl1StateLkpSramUncorrectable.Size()

	if fldName == "Bndl1StateLkpSramCorrectable" {
		return offset
	}
	offset += mtr.Bndl1StateLkpSramCorrectable.Size()

	return offset
}

// SetPktMemUncorrectable sets cunter in shared memory
func (mtr *Ppappa0inteccMetrics) SetPktMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktMemUncorrectable"))
	return nil
}

// SetPktMemCorrectable sets cunter in shared memory
func (mtr *Ppappa0inteccMetrics) SetPktMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktMemCorrectable"))
	return nil
}

// SetBndl0StateLkpSramUncorrectable sets cunter in shared memory
func (mtr *Ppappa0inteccMetrics) SetBndl0StateLkpSramUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Bndl0StateLkpSramUncorrectable"))
	return nil
}

// SetBndl0StateLkpSramCorrectable sets cunter in shared memory
func (mtr *Ppappa0inteccMetrics) SetBndl0StateLkpSramCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Bndl0StateLkpSramCorrectable"))
	return nil
}

// SetBndl1StateLkpSramUncorrectable sets cunter in shared memory
func (mtr *Ppappa0inteccMetrics) SetBndl1StateLkpSramUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Bndl1StateLkpSramUncorrectable"))
	return nil
}

// SetBndl1StateLkpSramCorrectable sets cunter in shared memory
func (mtr *Ppappa0inteccMetrics) SetBndl1StateLkpSramCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Bndl1StateLkpSramCorrectable"))
	return nil
}

// Ppappa0inteccMetricsIterator is the iterator object
type Ppappa0inteccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0inteccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0inteccMetricsIterator) Next() *Ppappa0inteccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0inteccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0inteccMetricsIterator) Find(key uint64) (*Ppappa0inteccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0inteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0inteccMetricsIterator) Create(key uint64) (*Ppappa0inteccMetrics, error) {
	tmtr := &Ppappa0inteccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0inteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0inteccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0inteccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0inteccMetricsIterator returns an iterator
func NewPpappa0inteccMetricsIterator() (*Ppappa0inteccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0inteccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0inteccMetricsIterator{iter: iter}, nil
}

type Ppappa0intfifo1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Pe0QctlFfOvflow metrics.Counter

	Pe1QctlFfOvflow metrics.Counter

	Pe2QctlFfOvflow metrics.Counter

	Pe3QctlFfOvflow metrics.Counter

	Pe4QctlFfOvflow metrics.Counter

	Pe5QctlFfOvflow metrics.Counter

	Pe6QctlFfOvflow metrics.Counter

	Pe7QctlFfOvflow metrics.Counter

	Pe8QctlFfOvflow metrics.Counter

	Pe9QctlFfOvflow metrics.Counter

	Pe0PhvFfOvflow metrics.Counter

	Pe1PhvFfOvflow metrics.Counter

	Pe2PhvFfOvflow metrics.Counter

	Pe3PhvFfOvflow metrics.Counter

	Pe4PhvFfOvflow metrics.Counter

	Pe5PhvFfOvflow metrics.Counter

	Pe6PhvFfOvflow metrics.Counter

	Pe7PhvFfOvflow metrics.Counter

	Pe8PhvFfOvflow metrics.Counter

	Pe9PhvFfOvflow metrics.Counter

	Pe0OhiFfOvflow metrics.Counter

	Pe1OhiFfOvflow metrics.Counter

	Pe2OhiFfOvflow metrics.Counter

	Pe3OhiFfOvflow metrics.Counter

	Pe4OhiFfOvflow metrics.Counter

	Pe5OhiFfOvflow metrics.Counter

	Pe6OhiFfOvflow metrics.Counter

	Pe7OhiFfOvflow metrics.Counter

	Pe8OhiFfOvflow metrics.Counter

	Pe9OhiFfOvflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intfifo1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intfifo1Metrics) Size() int {
	sz := 0

	sz += mtr.Pe0QctlFfOvflow.Size()

	sz += mtr.Pe1QctlFfOvflow.Size()

	sz += mtr.Pe2QctlFfOvflow.Size()

	sz += mtr.Pe3QctlFfOvflow.Size()

	sz += mtr.Pe4QctlFfOvflow.Size()

	sz += mtr.Pe5QctlFfOvflow.Size()

	sz += mtr.Pe6QctlFfOvflow.Size()

	sz += mtr.Pe7QctlFfOvflow.Size()

	sz += mtr.Pe8QctlFfOvflow.Size()

	sz += mtr.Pe9QctlFfOvflow.Size()

	sz += mtr.Pe0PhvFfOvflow.Size()

	sz += mtr.Pe1PhvFfOvflow.Size()

	sz += mtr.Pe2PhvFfOvflow.Size()

	sz += mtr.Pe3PhvFfOvflow.Size()

	sz += mtr.Pe4PhvFfOvflow.Size()

	sz += mtr.Pe5PhvFfOvflow.Size()

	sz += mtr.Pe6PhvFfOvflow.Size()

	sz += mtr.Pe7PhvFfOvflow.Size()

	sz += mtr.Pe8PhvFfOvflow.Size()

	sz += mtr.Pe9PhvFfOvflow.Size()

	sz += mtr.Pe0OhiFfOvflow.Size()

	sz += mtr.Pe1OhiFfOvflow.Size()

	sz += mtr.Pe2OhiFfOvflow.Size()

	sz += mtr.Pe3OhiFfOvflow.Size()

	sz += mtr.Pe4OhiFfOvflow.Size()

	sz += mtr.Pe5OhiFfOvflow.Size()

	sz += mtr.Pe6OhiFfOvflow.Size()

	sz += mtr.Pe7OhiFfOvflow.Size()

	sz += mtr.Pe8OhiFfOvflow.Size()

	sz += mtr.Pe9OhiFfOvflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intfifo1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Pe0QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe0QctlFfOvflow.Size()

	mtr.Pe1QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe1QctlFfOvflow.Size()

	mtr.Pe2QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe2QctlFfOvflow.Size()

	mtr.Pe3QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe3QctlFfOvflow.Size()

	mtr.Pe4QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe4QctlFfOvflow.Size()

	mtr.Pe5QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe5QctlFfOvflow.Size()

	mtr.Pe6QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe6QctlFfOvflow.Size()

	mtr.Pe7QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe7QctlFfOvflow.Size()

	mtr.Pe8QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe8QctlFfOvflow.Size()

	mtr.Pe9QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe9QctlFfOvflow.Size()

	mtr.Pe0PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe0PhvFfOvflow.Size()

	mtr.Pe1PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe1PhvFfOvflow.Size()

	mtr.Pe2PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe2PhvFfOvflow.Size()

	mtr.Pe3PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe3PhvFfOvflow.Size()

	mtr.Pe4PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe4PhvFfOvflow.Size()

	mtr.Pe5PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe5PhvFfOvflow.Size()

	mtr.Pe6PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe6PhvFfOvflow.Size()

	mtr.Pe7PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe7PhvFfOvflow.Size()

	mtr.Pe8PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe8PhvFfOvflow.Size()

	mtr.Pe9PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe9PhvFfOvflow.Size()

	mtr.Pe0OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe0OhiFfOvflow.Size()

	mtr.Pe1OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe1OhiFfOvflow.Size()

	mtr.Pe2OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe2OhiFfOvflow.Size()

	mtr.Pe3OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe3OhiFfOvflow.Size()

	mtr.Pe4OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe4OhiFfOvflow.Size()

	mtr.Pe5OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe5OhiFfOvflow.Size()

	mtr.Pe6OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe6OhiFfOvflow.Size()

	mtr.Pe7OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe7OhiFfOvflow.Size()

	mtr.Pe8OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe8OhiFfOvflow.Size()

	mtr.Pe9OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe9OhiFfOvflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intfifo1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Pe0QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe0QctlFfOvflow.Size()

	if fldName == "Pe1QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe1QctlFfOvflow.Size()

	if fldName == "Pe2QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe2QctlFfOvflow.Size()

	if fldName == "Pe3QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe3QctlFfOvflow.Size()

	if fldName == "Pe4QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe4QctlFfOvflow.Size()

	if fldName == "Pe5QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe5QctlFfOvflow.Size()

	if fldName == "Pe6QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe6QctlFfOvflow.Size()

	if fldName == "Pe7QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe7QctlFfOvflow.Size()

	if fldName == "Pe8QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe8QctlFfOvflow.Size()

	if fldName == "Pe9QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe9QctlFfOvflow.Size()

	if fldName == "Pe0PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe0PhvFfOvflow.Size()

	if fldName == "Pe1PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe1PhvFfOvflow.Size()

	if fldName == "Pe2PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe2PhvFfOvflow.Size()

	if fldName == "Pe3PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe3PhvFfOvflow.Size()

	if fldName == "Pe4PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe4PhvFfOvflow.Size()

	if fldName == "Pe5PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe5PhvFfOvflow.Size()

	if fldName == "Pe6PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe6PhvFfOvflow.Size()

	if fldName == "Pe7PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe7PhvFfOvflow.Size()

	if fldName == "Pe8PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe8PhvFfOvflow.Size()

	if fldName == "Pe9PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe9PhvFfOvflow.Size()

	if fldName == "Pe0OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe0OhiFfOvflow.Size()

	if fldName == "Pe1OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe1OhiFfOvflow.Size()

	if fldName == "Pe2OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe2OhiFfOvflow.Size()

	if fldName == "Pe3OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe3OhiFfOvflow.Size()

	if fldName == "Pe4OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe4OhiFfOvflow.Size()

	if fldName == "Pe5OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe5OhiFfOvflow.Size()

	if fldName == "Pe6OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe6OhiFfOvflow.Size()

	if fldName == "Pe7OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe7OhiFfOvflow.Size()

	if fldName == "Pe8OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe8OhiFfOvflow.Size()

	if fldName == "Pe9OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe9OhiFfOvflow.Size()

	return offset
}

// SetPe0QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe0QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe0QctlFfOvflow"))
	return nil
}

// SetPe1QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe1QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe1QctlFfOvflow"))
	return nil
}

// SetPe2QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe2QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe2QctlFfOvflow"))
	return nil
}

// SetPe3QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe3QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe3QctlFfOvflow"))
	return nil
}

// SetPe4QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe4QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe4QctlFfOvflow"))
	return nil
}

// SetPe5QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe5QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe5QctlFfOvflow"))
	return nil
}

// SetPe6QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe6QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe6QctlFfOvflow"))
	return nil
}

// SetPe7QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe7QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe7QctlFfOvflow"))
	return nil
}

// SetPe8QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe8QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe8QctlFfOvflow"))
	return nil
}

// SetPe9QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe9QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe9QctlFfOvflow"))
	return nil
}

// SetPe0PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe0PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe0PhvFfOvflow"))
	return nil
}

// SetPe1PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe1PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe1PhvFfOvflow"))
	return nil
}

// SetPe2PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe2PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe2PhvFfOvflow"))
	return nil
}

// SetPe3PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe3PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe3PhvFfOvflow"))
	return nil
}

// SetPe4PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe4PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe4PhvFfOvflow"))
	return nil
}

// SetPe5PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe5PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe5PhvFfOvflow"))
	return nil
}

// SetPe6PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe6PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe6PhvFfOvflow"))
	return nil
}

// SetPe7PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe7PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe7PhvFfOvflow"))
	return nil
}

// SetPe8PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe8PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe8PhvFfOvflow"))
	return nil
}

// SetPe9PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe9PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe9PhvFfOvflow"))
	return nil
}

// SetPe0OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe0OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe0OhiFfOvflow"))
	return nil
}

// SetPe1OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe1OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe1OhiFfOvflow"))
	return nil
}

// SetPe2OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe2OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe2OhiFfOvflow"))
	return nil
}

// SetPe3OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe3OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe3OhiFfOvflow"))
	return nil
}

// SetPe4OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe4OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe4OhiFfOvflow"))
	return nil
}

// SetPe5OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe5OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe5OhiFfOvflow"))
	return nil
}

// SetPe6OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe6OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe6OhiFfOvflow"))
	return nil
}

// SetPe7OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe7OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe7OhiFfOvflow"))
	return nil
}

// SetPe8OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe8OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe8OhiFfOvflow"))
	return nil
}

// SetPe9OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo1Metrics) SetPe9OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe9OhiFfOvflow"))
	return nil
}

// Ppappa0intfifo1MetricsIterator is the iterator object
type Ppappa0intfifo1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intfifo1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intfifo1MetricsIterator) Next() *Ppappa0intfifo1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intfifo1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intfifo1MetricsIterator) Find(key uint64) (*Ppappa0intfifo1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intfifo1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intfifo1MetricsIterator) Create(key uint64) (*Ppappa0intfifo1Metrics, error) {
	tmtr := &Ppappa0intfifo1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intfifo1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intfifo1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intfifo1MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intfifo1MetricsIterator returns an iterator
func NewPpappa0intfifo1MetricsIterator() (*Ppappa0intfifo1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intfifo1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intfifo1MetricsIterator{iter: iter}, nil
}

type Ppappa0intfifo2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DpIfFfOvflow metrics.Counter

	OhiIfFfOvflow metrics.Counter

	MaIfFfOvflow metrics.Counter

	PbIfFfOvflow metrics.Counter

	PktMtuFfOvflow metrics.Counter

	OutphvFfOvflow metrics.Counter

	OutohiFfOvflow metrics.Counter

	ChkFfOvflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intfifo2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intfifo2Metrics) Size() int {
	sz := 0

	sz += mtr.DpIfFfOvflow.Size()

	sz += mtr.OhiIfFfOvflow.Size()

	sz += mtr.MaIfFfOvflow.Size()

	sz += mtr.PbIfFfOvflow.Size()

	sz += mtr.PktMtuFfOvflow.Size()

	sz += mtr.OutphvFfOvflow.Size()

	sz += mtr.OutohiFfOvflow.Size()

	sz += mtr.ChkFfOvflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intfifo2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.DpIfFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DpIfFfOvflow.Size()

	mtr.OhiIfFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.OhiIfFfOvflow.Size()

	mtr.MaIfFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.MaIfFfOvflow.Size()

	mtr.PbIfFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PbIfFfOvflow.Size()

	mtr.PktMtuFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktMtuFfOvflow.Size()

	mtr.OutphvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.OutphvFfOvflow.Size()

	mtr.OutohiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.OutohiFfOvflow.Size()

	mtr.ChkFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ChkFfOvflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intfifo2Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "DpIfFfOvflow" {
		return offset
	}
	offset += mtr.DpIfFfOvflow.Size()

	if fldName == "OhiIfFfOvflow" {
		return offset
	}
	offset += mtr.OhiIfFfOvflow.Size()

	if fldName == "MaIfFfOvflow" {
		return offset
	}
	offset += mtr.MaIfFfOvflow.Size()

	if fldName == "PbIfFfOvflow" {
		return offset
	}
	offset += mtr.PbIfFfOvflow.Size()

	if fldName == "PktMtuFfOvflow" {
		return offset
	}
	offset += mtr.PktMtuFfOvflow.Size()

	if fldName == "OutphvFfOvflow" {
		return offset
	}
	offset += mtr.OutphvFfOvflow.Size()

	if fldName == "OutohiFfOvflow" {
		return offset
	}
	offset += mtr.OutohiFfOvflow.Size()

	if fldName == "ChkFfOvflow" {
		return offset
	}
	offset += mtr.ChkFfOvflow.Size()

	return offset
}

// SetDpIfFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo2Metrics) SetDpIfFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DpIfFfOvflow"))
	return nil
}

// SetOhiIfFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo2Metrics) SetOhiIfFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiIfFfOvflow"))
	return nil
}

// SetMaIfFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo2Metrics) SetMaIfFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaIfFfOvflow"))
	return nil
}

// SetPbIfFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo2Metrics) SetPbIfFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbIfFfOvflow"))
	return nil
}

// SetPktMtuFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo2Metrics) SetPktMtuFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktMtuFfOvflow"))
	return nil
}

// SetOutphvFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo2Metrics) SetOutphvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OutphvFfOvflow"))
	return nil
}

// SetOutohiFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo2Metrics) SetOutohiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OutohiFfOvflow"))
	return nil
}

// SetChkFfOvflow sets cunter in shared memory
func (mtr *Ppappa0intfifo2Metrics) SetChkFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ChkFfOvflow"))
	return nil
}

// Ppappa0intfifo2MetricsIterator is the iterator object
type Ppappa0intfifo2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intfifo2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intfifo2MetricsIterator) Next() *Ppappa0intfifo2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intfifo2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intfifo2MetricsIterator) Find(key uint64) (*Ppappa0intfifo2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intfifo2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intfifo2MetricsIterator) Create(key uint64) (*Ppappa0intfifo2Metrics, error) {
	tmtr := &Ppappa0intfifo2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intfifo2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intfifo2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intfifo2MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intfifo2MetricsIterator returns an iterator
func NewPpappa0intfifo2MetricsIterator() (*Ppappa0intfifo2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intfifo2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intfifo2MetricsIterator{iter: iter}, nil
}

type Ppappa0intpe0Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intpe0Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intpe0Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intpe0Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intpe0Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa0intpe0Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa0intpe0Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa0intpe0Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa0intpe0Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa0intpe0Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa0intpe0Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa0intpe0Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa0intpe0Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa0intpe0Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa0intpe0Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa0intpe0MetricsIterator is the iterator object
type Ppappa0intpe0MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intpe0MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intpe0MetricsIterator) Next() *Ppappa0intpe0Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intpe0Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intpe0MetricsIterator) Find(key uint64) (*Ppappa0intpe0Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intpe0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intpe0MetricsIterator) Create(key uint64) (*Ppappa0intpe0Metrics, error) {
	tmtr := &Ppappa0intpe0Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intpe0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intpe0MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intpe0MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intpe0MetricsIterator returns an iterator
func NewPpappa0intpe0MetricsIterator() (*Ppappa0intpe0MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intpe0Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intpe0MetricsIterator{iter: iter}, nil
}

type Ppappa0intpe1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intpe1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intpe1Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intpe1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intpe1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa0intpe1Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa0intpe1Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa0intpe1Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa0intpe1Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa0intpe1Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa0intpe1Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa0intpe1Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa0intpe1Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa0intpe1Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa0intpe1Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa0intpe1MetricsIterator is the iterator object
type Ppappa0intpe1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intpe1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intpe1MetricsIterator) Next() *Ppappa0intpe1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intpe1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intpe1MetricsIterator) Find(key uint64) (*Ppappa0intpe1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intpe1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intpe1MetricsIterator) Create(key uint64) (*Ppappa0intpe1Metrics, error) {
	tmtr := &Ppappa0intpe1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intpe1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intpe1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intpe1MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intpe1MetricsIterator returns an iterator
func NewPpappa0intpe1MetricsIterator() (*Ppappa0intpe1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intpe1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intpe1MetricsIterator{iter: iter}, nil
}

type Ppappa0intpe2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intpe2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intpe2Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intpe2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intpe2Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa0intpe2Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa0intpe2Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa0intpe2Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa0intpe2Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa0intpe2Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa0intpe2Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa0intpe2Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa0intpe2Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa0intpe2Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa0intpe2Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa0intpe2MetricsIterator is the iterator object
type Ppappa0intpe2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intpe2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intpe2MetricsIterator) Next() *Ppappa0intpe2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intpe2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intpe2MetricsIterator) Find(key uint64) (*Ppappa0intpe2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intpe2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intpe2MetricsIterator) Create(key uint64) (*Ppappa0intpe2Metrics, error) {
	tmtr := &Ppappa0intpe2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intpe2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intpe2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intpe2MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intpe2MetricsIterator returns an iterator
func NewPpappa0intpe2MetricsIterator() (*Ppappa0intpe2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intpe2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intpe2MetricsIterator{iter: iter}, nil
}

type Ppappa0intpe3Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intpe3Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intpe3Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intpe3Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intpe3Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa0intpe3Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa0intpe3Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa0intpe3Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa0intpe3Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa0intpe3Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa0intpe3Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa0intpe3Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa0intpe3Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa0intpe3Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa0intpe3Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa0intpe3MetricsIterator is the iterator object
type Ppappa0intpe3MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intpe3MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intpe3MetricsIterator) Next() *Ppappa0intpe3Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intpe3Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intpe3MetricsIterator) Find(key uint64) (*Ppappa0intpe3Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intpe3Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intpe3MetricsIterator) Create(key uint64) (*Ppappa0intpe3Metrics, error) {
	tmtr := &Ppappa0intpe3Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intpe3Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intpe3MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intpe3MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intpe3MetricsIterator returns an iterator
func NewPpappa0intpe3MetricsIterator() (*Ppappa0intpe3MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intpe3Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intpe3MetricsIterator{iter: iter}, nil
}

type Ppappa0intpe4Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intpe4Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intpe4Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intpe4Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intpe4Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa0intpe4Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa0intpe4Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa0intpe4Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa0intpe4Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa0intpe4Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa0intpe4Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa0intpe4Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa0intpe4Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa0intpe4Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa0intpe4Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa0intpe4MetricsIterator is the iterator object
type Ppappa0intpe4MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intpe4MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intpe4MetricsIterator) Next() *Ppappa0intpe4Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intpe4Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intpe4MetricsIterator) Find(key uint64) (*Ppappa0intpe4Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intpe4Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intpe4MetricsIterator) Create(key uint64) (*Ppappa0intpe4Metrics, error) {
	tmtr := &Ppappa0intpe4Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intpe4Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intpe4MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intpe4MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intpe4MetricsIterator returns an iterator
func NewPpappa0intpe4MetricsIterator() (*Ppappa0intpe4MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intpe4Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intpe4MetricsIterator{iter: iter}, nil
}

type Ppappa0intpe5Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intpe5Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intpe5Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intpe5Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intpe5Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa0intpe5Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa0intpe5Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa0intpe5Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa0intpe5Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa0intpe5Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa0intpe5Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa0intpe5Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa0intpe5Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa0intpe5Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa0intpe5Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa0intpe5MetricsIterator is the iterator object
type Ppappa0intpe5MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intpe5MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intpe5MetricsIterator) Next() *Ppappa0intpe5Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intpe5Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intpe5MetricsIterator) Find(key uint64) (*Ppappa0intpe5Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intpe5Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intpe5MetricsIterator) Create(key uint64) (*Ppappa0intpe5Metrics, error) {
	tmtr := &Ppappa0intpe5Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intpe5Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intpe5MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intpe5MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intpe5MetricsIterator returns an iterator
func NewPpappa0intpe5MetricsIterator() (*Ppappa0intpe5MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intpe5Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intpe5MetricsIterator{iter: iter}, nil
}

type Ppappa0intpe6Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intpe6Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intpe6Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intpe6Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intpe6Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa0intpe6Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa0intpe6Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa0intpe6Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa0intpe6Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa0intpe6Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa0intpe6Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa0intpe6Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa0intpe6Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa0intpe6Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa0intpe6Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa0intpe6MetricsIterator is the iterator object
type Ppappa0intpe6MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intpe6MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intpe6MetricsIterator) Next() *Ppappa0intpe6Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intpe6Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intpe6MetricsIterator) Find(key uint64) (*Ppappa0intpe6Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intpe6Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intpe6MetricsIterator) Create(key uint64) (*Ppappa0intpe6Metrics, error) {
	tmtr := &Ppappa0intpe6Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intpe6Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intpe6MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intpe6MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intpe6MetricsIterator returns an iterator
func NewPpappa0intpe6MetricsIterator() (*Ppappa0intpe6MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intpe6Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intpe6MetricsIterator{iter: iter}, nil
}

type Ppappa0intpe7Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intpe7Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intpe7Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intpe7Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intpe7Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa0intpe7Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa0intpe7Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa0intpe7Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa0intpe7Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa0intpe7Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa0intpe7Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa0intpe7Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa0intpe7Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa0intpe7Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa0intpe7Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa0intpe7MetricsIterator is the iterator object
type Ppappa0intpe7MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intpe7MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intpe7MetricsIterator) Next() *Ppappa0intpe7Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intpe7Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intpe7MetricsIterator) Find(key uint64) (*Ppappa0intpe7Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intpe7Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intpe7MetricsIterator) Create(key uint64) (*Ppappa0intpe7Metrics, error) {
	tmtr := &Ppappa0intpe7Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intpe7Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intpe7MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intpe7MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intpe7MetricsIterator returns an iterator
func NewPpappa0intpe7MetricsIterator() (*Ppappa0intpe7MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intpe7Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intpe7MetricsIterator{iter: iter}, nil
}

type Ppappa0intpe8Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intpe8Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intpe8Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intpe8Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intpe8Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa0intpe8Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa0intpe8Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa0intpe8Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa0intpe8Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa0intpe8Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa0intpe8Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa0intpe8Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa0intpe8Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa0intpe8Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa0intpe8Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa0intpe8MetricsIterator is the iterator object
type Ppappa0intpe8MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intpe8MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intpe8MetricsIterator) Next() *Ppappa0intpe8Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intpe8Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intpe8MetricsIterator) Find(key uint64) (*Ppappa0intpe8Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intpe8Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intpe8MetricsIterator) Create(key uint64) (*Ppappa0intpe8Metrics, error) {
	tmtr := &Ppappa0intpe8Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intpe8Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intpe8MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intpe8MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intpe8MetricsIterator returns an iterator
func NewPpappa0intpe8MetricsIterator() (*Ppappa0intpe8MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intpe8Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intpe8MetricsIterator{iter: iter}, nil
}

type Ppappa0intpe9Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intpe9Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intpe9Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intpe9Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intpe9Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa0intpe9Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa0intpe9Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa0intpe9Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa0intpe9Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa0intpe9Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa0intpe9Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa0intpe9Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa0intpe9Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa0intpe9Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa0intpe9Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa0intpe9MetricsIterator is the iterator object
type Ppappa0intpe9MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intpe9MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intpe9MetricsIterator) Next() *Ppappa0intpe9Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intpe9Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intpe9MetricsIterator) Find(key uint64) (*Ppappa0intpe9Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intpe9Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intpe9MetricsIterator) Create(key uint64) (*Ppappa0intpe9Metrics, error) {
	tmtr := &Ppappa0intpe9Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intpe9Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intpe9MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intpe9MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intpe9MetricsIterator returns an iterator
func NewPpappa0intpe9MetricsIterator() (*Ppappa0intpe9MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intpe9Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intpe9MetricsIterator{iter: iter}, nil
}

type Ppappa0intpaMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	ReorderPhvNotSop metrics.Counter

	Csum0NotEnufBytes metrics.Counter

	Csum0StartMoreThanEndOffset metrics.Counter

	Csum1NotEnufBytes metrics.Counter

	Csum1StartMoreThanEndOffset metrics.Counter

	Csum2NotEnufBytes metrics.Counter

	Csum2StartMoreThanEndOffset metrics.Counter

	Csum3NotEnufBytes metrics.Counter

	Csum3StartMoreThanEndOffset metrics.Counter

	Csum4NotEnufBytes metrics.Counter

	Csum4StartMoreThanEndOffset metrics.Counter

	CrcNotEnufBytes metrics.Counter

	CrcStartMoreThanEndOffset metrics.Counter

	ChkrPhvSingleFlit metrics.Counter

	ChkrPhvNotSop metrics.Counter

	ChkrPktNotSop metrics.Counter

	ChkrSeqId metrics.Counter

	PreparseLine0NotSop metrics.Counter

	PreparseDffNotSop metrics.Counter

	PreparseDffOvflow metrics.Counter

	PreparseEffOvflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intpaMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intpaMetrics) Size() int {
	sz := 0

	sz += mtr.ReorderPhvNotSop.Size()

	sz += mtr.Csum0NotEnufBytes.Size()

	sz += mtr.Csum0StartMoreThanEndOffset.Size()

	sz += mtr.Csum1NotEnufBytes.Size()

	sz += mtr.Csum1StartMoreThanEndOffset.Size()

	sz += mtr.Csum2NotEnufBytes.Size()

	sz += mtr.Csum2StartMoreThanEndOffset.Size()

	sz += mtr.Csum3NotEnufBytes.Size()

	sz += mtr.Csum3StartMoreThanEndOffset.Size()

	sz += mtr.Csum4NotEnufBytes.Size()

	sz += mtr.Csum4StartMoreThanEndOffset.Size()

	sz += mtr.CrcNotEnufBytes.Size()

	sz += mtr.CrcStartMoreThanEndOffset.Size()

	sz += mtr.ChkrPhvSingleFlit.Size()

	sz += mtr.ChkrPhvNotSop.Size()

	sz += mtr.ChkrPktNotSop.Size()

	sz += mtr.ChkrSeqId.Size()

	sz += mtr.PreparseLine0NotSop.Size()

	sz += mtr.PreparseDffNotSop.Size()

	sz += mtr.PreparseDffOvflow.Size()

	sz += mtr.PreparseEffOvflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intpaMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.ReorderPhvNotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.ReorderPhvNotSop.Size()

	mtr.Csum0NotEnufBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum0NotEnufBytes.Size()

	mtr.Csum0StartMoreThanEndOffset = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum0StartMoreThanEndOffset.Size()

	mtr.Csum1NotEnufBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum1NotEnufBytes.Size()

	mtr.Csum1StartMoreThanEndOffset = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum1StartMoreThanEndOffset.Size()

	mtr.Csum2NotEnufBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum2NotEnufBytes.Size()

	mtr.Csum2StartMoreThanEndOffset = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum2StartMoreThanEndOffset.Size()

	mtr.Csum3NotEnufBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum3NotEnufBytes.Size()

	mtr.Csum3StartMoreThanEndOffset = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum3StartMoreThanEndOffset.Size()

	mtr.Csum4NotEnufBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum4NotEnufBytes.Size()

	mtr.Csum4StartMoreThanEndOffset = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum4StartMoreThanEndOffset.Size()

	mtr.CrcNotEnufBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.CrcNotEnufBytes.Size()

	mtr.CrcStartMoreThanEndOffset = mtr.metrics.GetCounter(offset)
	offset += mtr.CrcStartMoreThanEndOffset.Size()

	mtr.ChkrPhvSingleFlit = mtr.metrics.GetCounter(offset)
	offset += mtr.ChkrPhvSingleFlit.Size()

	mtr.ChkrPhvNotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.ChkrPhvNotSop.Size()

	mtr.ChkrPktNotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.ChkrPktNotSop.Size()

	mtr.ChkrSeqId = mtr.metrics.GetCounter(offset)
	offset += mtr.ChkrSeqId.Size()

	mtr.PreparseLine0NotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.PreparseLine0NotSop.Size()

	mtr.PreparseDffNotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.PreparseDffNotSop.Size()

	mtr.PreparseDffOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PreparseDffOvflow.Size()

	mtr.PreparseEffOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PreparseEffOvflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intpaMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "ReorderPhvNotSop" {
		return offset
	}
	offset += mtr.ReorderPhvNotSop.Size()

	if fldName == "Csum0NotEnufBytes" {
		return offset
	}
	offset += mtr.Csum0NotEnufBytes.Size()

	if fldName == "Csum0StartMoreThanEndOffset" {
		return offset
	}
	offset += mtr.Csum0StartMoreThanEndOffset.Size()

	if fldName == "Csum1NotEnufBytes" {
		return offset
	}
	offset += mtr.Csum1NotEnufBytes.Size()

	if fldName == "Csum1StartMoreThanEndOffset" {
		return offset
	}
	offset += mtr.Csum1StartMoreThanEndOffset.Size()

	if fldName == "Csum2NotEnufBytes" {
		return offset
	}
	offset += mtr.Csum2NotEnufBytes.Size()

	if fldName == "Csum2StartMoreThanEndOffset" {
		return offset
	}
	offset += mtr.Csum2StartMoreThanEndOffset.Size()

	if fldName == "Csum3NotEnufBytes" {
		return offset
	}
	offset += mtr.Csum3NotEnufBytes.Size()

	if fldName == "Csum3StartMoreThanEndOffset" {
		return offset
	}
	offset += mtr.Csum3StartMoreThanEndOffset.Size()

	if fldName == "Csum4NotEnufBytes" {
		return offset
	}
	offset += mtr.Csum4NotEnufBytes.Size()

	if fldName == "Csum4StartMoreThanEndOffset" {
		return offset
	}
	offset += mtr.Csum4StartMoreThanEndOffset.Size()

	if fldName == "CrcNotEnufBytes" {
		return offset
	}
	offset += mtr.CrcNotEnufBytes.Size()

	if fldName == "CrcStartMoreThanEndOffset" {
		return offset
	}
	offset += mtr.CrcStartMoreThanEndOffset.Size()

	if fldName == "ChkrPhvSingleFlit" {
		return offset
	}
	offset += mtr.ChkrPhvSingleFlit.Size()

	if fldName == "ChkrPhvNotSop" {
		return offset
	}
	offset += mtr.ChkrPhvNotSop.Size()

	if fldName == "ChkrPktNotSop" {
		return offset
	}
	offset += mtr.ChkrPktNotSop.Size()

	if fldName == "ChkrSeqId" {
		return offset
	}
	offset += mtr.ChkrSeqId.Size()

	if fldName == "PreparseLine0NotSop" {
		return offset
	}
	offset += mtr.PreparseLine0NotSop.Size()

	if fldName == "PreparseDffNotSop" {
		return offset
	}
	offset += mtr.PreparseDffNotSop.Size()

	if fldName == "PreparseDffOvflow" {
		return offset
	}
	offset += mtr.PreparseDffOvflow.Size()

	if fldName == "PreparseEffOvflow" {
		return offset
	}
	offset += mtr.PreparseEffOvflow.Size()

	return offset
}

// SetReorderPhvNotSop sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetReorderPhvNotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ReorderPhvNotSop"))
	return nil
}

// SetCsum0NotEnufBytes sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetCsum0NotEnufBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum0NotEnufBytes"))
	return nil
}

// SetCsum0StartMoreThanEndOffset sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetCsum0StartMoreThanEndOffset(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum0StartMoreThanEndOffset"))
	return nil
}

// SetCsum1NotEnufBytes sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetCsum1NotEnufBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum1NotEnufBytes"))
	return nil
}

// SetCsum1StartMoreThanEndOffset sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetCsum1StartMoreThanEndOffset(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum1StartMoreThanEndOffset"))
	return nil
}

// SetCsum2NotEnufBytes sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetCsum2NotEnufBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum2NotEnufBytes"))
	return nil
}

// SetCsum2StartMoreThanEndOffset sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetCsum2StartMoreThanEndOffset(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum2StartMoreThanEndOffset"))
	return nil
}

// SetCsum3NotEnufBytes sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetCsum3NotEnufBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum3NotEnufBytes"))
	return nil
}

// SetCsum3StartMoreThanEndOffset sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetCsum3StartMoreThanEndOffset(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum3StartMoreThanEndOffset"))
	return nil
}

// SetCsum4NotEnufBytes sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetCsum4NotEnufBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum4NotEnufBytes"))
	return nil
}

// SetCsum4StartMoreThanEndOffset sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetCsum4StartMoreThanEndOffset(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum4StartMoreThanEndOffset"))
	return nil
}

// SetCrcNotEnufBytes sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetCrcNotEnufBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CrcNotEnufBytes"))
	return nil
}

// SetCrcStartMoreThanEndOffset sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetCrcStartMoreThanEndOffset(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CrcStartMoreThanEndOffset"))
	return nil
}

// SetChkrPhvSingleFlit sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetChkrPhvSingleFlit(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ChkrPhvSingleFlit"))
	return nil
}

// SetChkrPhvNotSop sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetChkrPhvNotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ChkrPhvNotSop"))
	return nil
}

// SetChkrPktNotSop sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetChkrPktNotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ChkrPktNotSop"))
	return nil
}

// SetChkrSeqId sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetChkrSeqId(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ChkrSeqId"))
	return nil
}

// SetPreparseLine0NotSop sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetPreparseLine0NotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PreparseLine0NotSop"))
	return nil
}

// SetPreparseDffNotSop sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetPreparseDffNotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PreparseDffNotSop"))
	return nil
}

// SetPreparseDffOvflow sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetPreparseDffOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PreparseDffOvflow"))
	return nil
}

// SetPreparseEffOvflow sets cunter in shared memory
func (mtr *Ppappa0intpaMetrics) SetPreparseEffOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PreparseEffOvflow"))
	return nil
}

// Ppappa0intpaMetricsIterator is the iterator object
type Ppappa0intpaMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intpaMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intpaMetricsIterator) Next() *Ppappa0intpaMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intpaMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intpaMetricsIterator) Find(key uint64) (*Ppappa0intpaMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intpaMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intpaMetricsIterator) Create(key uint64) (*Ppappa0intpaMetrics, error) {
	tmtr := &Ppappa0intpaMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intpaMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intpaMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intpaMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intpaMetricsIterator returns an iterator
func NewPpappa0intpaMetricsIterator() (*Ppappa0intpaMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intpaMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intpaMetricsIterator{iter: iter}, nil
}

type Ppappa0intswphvmemMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	EccUncorrectable metrics.Counter

	EccCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intswphvmemMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intswphvmemMetrics) Size() int {
	sz := 0

	sz += mtr.EccUncorrectable.Size()

	sz += mtr.EccCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intswphvmemMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.EccUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccUncorrectable.Size()

	mtr.EccCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intswphvmemMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "EccUncorrectable" {
		return offset
	}
	offset += mtr.EccUncorrectable.Size()

	if fldName == "EccCorrectable" {
		return offset
	}
	offset += mtr.EccCorrectable.Size()

	return offset
}

// SetEccUncorrectable sets cunter in shared memory
func (mtr *Ppappa0intswphvmemMetrics) SetEccUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccUncorrectable"))
	return nil
}

// SetEccCorrectable sets cunter in shared memory
func (mtr *Ppappa0intswphvmemMetrics) SetEccCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccCorrectable"))
	return nil
}

// Ppappa0intswphvmemMetricsIterator is the iterator object
type Ppappa0intswphvmemMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intswphvmemMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intswphvmemMetricsIterator) Next() *Ppappa0intswphvmemMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intswphvmemMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intswphvmemMetricsIterator) Find(key uint64) (*Ppappa0intswphvmemMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intswphvmemMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intswphvmemMetricsIterator) Create(key uint64) (*Ppappa0intswphvmemMetrics, error) {
	tmtr := &Ppappa0intswphvmemMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intswphvmemMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intswphvmemMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intswphvmemMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intswphvmemMetricsIterator returns an iterator
func NewPpappa0intswphvmemMetricsIterator() (*Ppappa0intswphvmemMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intswphvmemMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intswphvmemMetricsIterator{iter: iter}, nil
}

type Ppappa0intintfMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PbErr metrics.Counter

	PbSopErr metrics.Counter

	PbEopErr metrics.Counter

	DpErr metrics.Counter

	DpSopErr metrics.Counter

	DpEopErr metrics.Counter

	OhiErr metrics.Counter

	OhiSopErr metrics.Counter

	OhiEopErr metrics.Counter

	SwPhvErr metrics.Counter

	SwPhvSopErr metrics.Counter

	SwPhvEopErr metrics.Counter

	MaErr metrics.Counter

	MaSopErr metrics.Counter

	MaEopErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa0intintfMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa0intintfMetrics) Size() int {
	sz := 0

	sz += mtr.PbErr.Size()

	sz += mtr.PbSopErr.Size()

	sz += mtr.PbEopErr.Size()

	sz += mtr.DpErr.Size()

	sz += mtr.DpSopErr.Size()

	sz += mtr.DpEopErr.Size()

	sz += mtr.OhiErr.Size()

	sz += mtr.OhiSopErr.Size()

	sz += mtr.OhiEopErr.Size()

	sz += mtr.SwPhvErr.Size()

	sz += mtr.SwPhvSopErr.Size()

	sz += mtr.SwPhvEopErr.Size()

	sz += mtr.MaErr.Size()

	sz += mtr.MaSopErr.Size()

	sz += mtr.MaEopErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa0intintfMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PbErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbErr.Size()

	mtr.PbSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbSopErr.Size()

	mtr.PbEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbEopErr.Size()

	mtr.DpErr = mtr.metrics.GetCounter(offset)
	offset += mtr.DpErr.Size()

	mtr.DpSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.DpSopErr.Size()

	mtr.DpEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.DpEopErr.Size()

	mtr.OhiErr = mtr.metrics.GetCounter(offset)
	offset += mtr.OhiErr.Size()

	mtr.OhiSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.OhiSopErr.Size()

	mtr.OhiEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.OhiEopErr.Size()

	mtr.SwPhvErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SwPhvErr.Size()

	mtr.SwPhvSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SwPhvSopErr.Size()

	mtr.SwPhvEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SwPhvEopErr.Size()

	mtr.MaErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaErr.Size()

	mtr.MaSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaSopErr.Size()

	mtr.MaEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaEopErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa0intintfMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PbErr" {
		return offset
	}
	offset += mtr.PbErr.Size()

	if fldName == "PbSopErr" {
		return offset
	}
	offset += mtr.PbSopErr.Size()

	if fldName == "PbEopErr" {
		return offset
	}
	offset += mtr.PbEopErr.Size()

	if fldName == "DpErr" {
		return offset
	}
	offset += mtr.DpErr.Size()

	if fldName == "DpSopErr" {
		return offset
	}
	offset += mtr.DpSopErr.Size()

	if fldName == "DpEopErr" {
		return offset
	}
	offset += mtr.DpEopErr.Size()

	if fldName == "OhiErr" {
		return offset
	}
	offset += mtr.OhiErr.Size()

	if fldName == "OhiSopErr" {
		return offset
	}
	offset += mtr.OhiSopErr.Size()

	if fldName == "OhiEopErr" {
		return offset
	}
	offset += mtr.OhiEopErr.Size()

	if fldName == "SwPhvErr" {
		return offset
	}
	offset += mtr.SwPhvErr.Size()

	if fldName == "SwPhvSopErr" {
		return offset
	}
	offset += mtr.SwPhvSopErr.Size()

	if fldName == "SwPhvEopErr" {
		return offset
	}
	offset += mtr.SwPhvEopErr.Size()

	if fldName == "MaErr" {
		return offset
	}
	offset += mtr.MaErr.Size()

	if fldName == "MaSopErr" {
		return offset
	}
	offset += mtr.MaSopErr.Size()

	if fldName == "MaEopErr" {
		return offset
	}
	offset += mtr.MaEopErr.Size()

	return offset
}

// SetPbErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetPbErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbErr"))
	return nil
}

// SetPbSopErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetPbSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbSopErr"))
	return nil
}

// SetPbEopErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetPbEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbEopErr"))
	return nil
}

// SetDpErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetDpErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DpErr"))
	return nil
}

// SetDpSopErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetDpSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DpSopErr"))
	return nil
}

// SetDpEopErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetDpEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DpEopErr"))
	return nil
}

// SetOhiErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetOhiErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiErr"))
	return nil
}

// SetOhiSopErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetOhiSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiSopErr"))
	return nil
}

// SetOhiEopErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetOhiEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiEopErr"))
	return nil
}

// SetSwPhvErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetSwPhvErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SwPhvErr"))
	return nil
}

// SetSwPhvSopErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetSwPhvSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SwPhvSopErr"))
	return nil
}

// SetSwPhvEopErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetSwPhvEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SwPhvEopErr"))
	return nil
}

// SetMaErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetMaErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaErr"))
	return nil
}

// SetMaSopErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetMaSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaSopErr"))
	return nil
}

// SetMaEopErr sets cunter in shared memory
func (mtr *Ppappa0intintfMetrics) SetMaEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaEopErr"))
	return nil
}

// Ppappa0intintfMetricsIterator is the iterator object
type Ppappa0intintfMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa0intintfMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa0intintfMetricsIterator) Next() *Ppappa0intintfMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa0intintfMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa0intintfMetricsIterator) Find(key uint64) (*Ppappa0intintfMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa0intintfMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa0intintfMetricsIterator) Create(key uint64) (*Ppappa0intintfMetrics, error) {
	tmtr := &Ppappa0intintfMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa0intintfMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa0intintfMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa0intintfMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa0intintfMetricsIterator returns an iterator
func NewPpappa0intintfMetricsIterator() (*Ppappa0intintfMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa0intintfMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa0intintfMetricsIterator{iter: iter}, nil
}

type Ppappa1intbndl0Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	StateLkpSrchHitAddrRangeErr metrics.Counter

	StateLkpSrchMiss metrics.Counter

	StateLkpSrchReqNotRdy metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intbndl0Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intbndl0Metrics) Size() int {
	sz := 0

	sz += mtr.StateLkpSrchHitAddrRangeErr.Size()

	sz += mtr.StateLkpSrchMiss.Size()

	sz += mtr.StateLkpSrchReqNotRdy.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intbndl0Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.StateLkpSrchHitAddrRangeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.StateLkpSrchHitAddrRangeErr.Size()

	mtr.StateLkpSrchMiss = mtr.metrics.GetCounter(offset)
	offset += mtr.StateLkpSrchMiss.Size()

	mtr.StateLkpSrchReqNotRdy = mtr.metrics.GetCounter(offset)
	offset += mtr.StateLkpSrchReqNotRdy.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intbndl0Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "StateLkpSrchHitAddrRangeErr" {
		return offset
	}
	offset += mtr.StateLkpSrchHitAddrRangeErr.Size()

	if fldName == "StateLkpSrchMiss" {
		return offset
	}
	offset += mtr.StateLkpSrchMiss.Size()

	if fldName == "StateLkpSrchReqNotRdy" {
		return offset
	}
	offset += mtr.StateLkpSrchReqNotRdy.Size()

	return offset
}

// SetStateLkpSrchHitAddrRangeErr sets cunter in shared memory
func (mtr *Ppappa1intbndl0Metrics) SetStateLkpSrchHitAddrRangeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StateLkpSrchHitAddrRangeErr"))
	return nil
}

// SetStateLkpSrchMiss sets cunter in shared memory
func (mtr *Ppappa1intbndl0Metrics) SetStateLkpSrchMiss(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StateLkpSrchMiss"))
	return nil
}

// SetStateLkpSrchReqNotRdy sets cunter in shared memory
func (mtr *Ppappa1intbndl0Metrics) SetStateLkpSrchReqNotRdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StateLkpSrchReqNotRdy"))
	return nil
}

// Ppappa1intbndl0MetricsIterator is the iterator object
type Ppappa1intbndl0MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intbndl0MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intbndl0MetricsIterator) Next() *Ppappa1intbndl0Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intbndl0Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intbndl0MetricsIterator) Find(key uint64) (*Ppappa1intbndl0Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intbndl0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intbndl0MetricsIterator) Create(key uint64) (*Ppappa1intbndl0Metrics, error) {
	tmtr := &Ppappa1intbndl0Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intbndl0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intbndl0MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intbndl0MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intbndl0MetricsIterator returns an iterator
func NewPpappa1intbndl0MetricsIterator() (*Ppappa1intbndl0MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intbndl0Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intbndl0MetricsIterator{iter: iter}, nil
}

type Ppappa1intbndl1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	StateLkpSrchHitAddrRangeErr metrics.Counter

	StateLkpSrchMiss metrics.Counter

	StateLkpSrchReqNotRdy metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intbndl1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intbndl1Metrics) Size() int {
	sz := 0

	sz += mtr.StateLkpSrchHitAddrRangeErr.Size()

	sz += mtr.StateLkpSrchMiss.Size()

	sz += mtr.StateLkpSrchReqNotRdy.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intbndl1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.StateLkpSrchHitAddrRangeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.StateLkpSrchHitAddrRangeErr.Size()

	mtr.StateLkpSrchMiss = mtr.metrics.GetCounter(offset)
	offset += mtr.StateLkpSrchMiss.Size()

	mtr.StateLkpSrchReqNotRdy = mtr.metrics.GetCounter(offset)
	offset += mtr.StateLkpSrchReqNotRdy.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intbndl1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "StateLkpSrchHitAddrRangeErr" {
		return offset
	}
	offset += mtr.StateLkpSrchHitAddrRangeErr.Size()

	if fldName == "StateLkpSrchMiss" {
		return offset
	}
	offset += mtr.StateLkpSrchMiss.Size()

	if fldName == "StateLkpSrchReqNotRdy" {
		return offset
	}
	offset += mtr.StateLkpSrchReqNotRdy.Size()

	return offset
}

// SetStateLkpSrchHitAddrRangeErr sets cunter in shared memory
func (mtr *Ppappa1intbndl1Metrics) SetStateLkpSrchHitAddrRangeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StateLkpSrchHitAddrRangeErr"))
	return nil
}

// SetStateLkpSrchMiss sets cunter in shared memory
func (mtr *Ppappa1intbndl1Metrics) SetStateLkpSrchMiss(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StateLkpSrchMiss"))
	return nil
}

// SetStateLkpSrchReqNotRdy sets cunter in shared memory
func (mtr *Ppappa1intbndl1Metrics) SetStateLkpSrchReqNotRdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StateLkpSrchReqNotRdy"))
	return nil
}

// Ppappa1intbndl1MetricsIterator is the iterator object
type Ppappa1intbndl1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intbndl1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intbndl1MetricsIterator) Next() *Ppappa1intbndl1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intbndl1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intbndl1MetricsIterator) Find(key uint64) (*Ppappa1intbndl1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intbndl1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intbndl1MetricsIterator) Create(key uint64) (*Ppappa1intbndl1Metrics, error) {
	tmtr := &Ppappa1intbndl1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intbndl1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intbndl1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intbndl1MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intbndl1MetricsIterator returns an iterator
func NewPpappa1intbndl1MetricsIterator() (*Ppappa1intbndl1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intbndl1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intbndl1MetricsIterator{iter: iter}, nil
}

type Ppappa1inteccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PktMemUncorrectable metrics.Counter

	PktMemCorrectable metrics.Counter

	Bndl0StateLkpSramUncorrectable metrics.Counter

	Bndl0StateLkpSramCorrectable metrics.Counter

	Bndl1StateLkpSramUncorrectable metrics.Counter

	Bndl1StateLkpSramCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1inteccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1inteccMetrics) Size() int {
	sz := 0

	sz += mtr.PktMemUncorrectable.Size()

	sz += mtr.PktMemCorrectable.Size()

	sz += mtr.Bndl0StateLkpSramUncorrectable.Size()

	sz += mtr.Bndl0StateLkpSramCorrectable.Size()

	sz += mtr.Bndl1StateLkpSramUncorrectable.Size()

	sz += mtr.Bndl1StateLkpSramCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1inteccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PktMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PktMemUncorrectable.Size()

	mtr.PktMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PktMemCorrectable.Size()

	mtr.Bndl0StateLkpSramUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Bndl0StateLkpSramUncorrectable.Size()

	mtr.Bndl0StateLkpSramCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Bndl0StateLkpSramCorrectable.Size()

	mtr.Bndl1StateLkpSramUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Bndl1StateLkpSramUncorrectable.Size()

	mtr.Bndl1StateLkpSramCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Bndl1StateLkpSramCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1inteccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PktMemUncorrectable" {
		return offset
	}
	offset += mtr.PktMemUncorrectable.Size()

	if fldName == "PktMemCorrectable" {
		return offset
	}
	offset += mtr.PktMemCorrectable.Size()

	if fldName == "Bndl0StateLkpSramUncorrectable" {
		return offset
	}
	offset += mtr.Bndl0StateLkpSramUncorrectable.Size()

	if fldName == "Bndl0StateLkpSramCorrectable" {
		return offset
	}
	offset += mtr.Bndl0StateLkpSramCorrectable.Size()

	if fldName == "Bndl1StateLkpSramUncorrectable" {
		return offset
	}
	offset += mtr.Bndl1StateLkpSramUncorrectable.Size()

	if fldName == "Bndl1StateLkpSramCorrectable" {
		return offset
	}
	offset += mtr.Bndl1StateLkpSramCorrectable.Size()

	return offset
}

// SetPktMemUncorrectable sets cunter in shared memory
func (mtr *Ppappa1inteccMetrics) SetPktMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktMemUncorrectable"))
	return nil
}

// SetPktMemCorrectable sets cunter in shared memory
func (mtr *Ppappa1inteccMetrics) SetPktMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktMemCorrectable"))
	return nil
}

// SetBndl0StateLkpSramUncorrectable sets cunter in shared memory
func (mtr *Ppappa1inteccMetrics) SetBndl0StateLkpSramUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Bndl0StateLkpSramUncorrectable"))
	return nil
}

// SetBndl0StateLkpSramCorrectable sets cunter in shared memory
func (mtr *Ppappa1inteccMetrics) SetBndl0StateLkpSramCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Bndl0StateLkpSramCorrectable"))
	return nil
}

// SetBndl1StateLkpSramUncorrectable sets cunter in shared memory
func (mtr *Ppappa1inteccMetrics) SetBndl1StateLkpSramUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Bndl1StateLkpSramUncorrectable"))
	return nil
}

// SetBndl1StateLkpSramCorrectable sets cunter in shared memory
func (mtr *Ppappa1inteccMetrics) SetBndl1StateLkpSramCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Bndl1StateLkpSramCorrectable"))
	return nil
}

// Ppappa1inteccMetricsIterator is the iterator object
type Ppappa1inteccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1inteccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1inteccMetricsIterator) Next() *Ppappa1inteccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1inteccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1inteccMetricsIterator) Find(key uint64) (*Ppappa1inteccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1inteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1inteccMetricsIterator) Create(key uint64) (*Ppappa1inteccMetrics, error) {
	tmtr := &Ppappa1inteccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1inteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1inteccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1inteccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1inteccMetricsIterator returns an iterator
func NewPpappa1inteccMetricsIterator() (*Ppappa1inteccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1inteccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1inteccMetricsIterator{iter: iter}, nil
}

type Ppappa1intfifo1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Pe0QctlFfOvflow metrics.Counter

	Pe1QctlFfOvflow metrics.Counter

	Pe2QctlFfOvflow metrics.Counter

	Pe3QctlFfOvflow metrics.Counter

	Pe4QctlFfOvflow metrics.Counter

	Pe5QctlFfOvflow metrics.Counter

	Pe6QctlFfOvflow metrics.Counter

	Pe7QctlFfOvflow metrics.Counter

	Pe8QctlFfOvflow metrics.Counter

	Pe9QctlFfOvflow metrics.Counter

	Pe0PhvFfOvflow metrics.Counter

	Pe1PhvFfOvflow metrics.Counter

	Pe2PhvFfOvflow metrics.Counter

	Pe3PhvFfOvflow metrics.Counter

	Pe4PhvFfOvflow metrics.Counter

	Pe5PhvFfOvflow metrics.Counter

	Pe6PhvFfOvflow metrics.Counter

	Pe7PhvFfOvflow metrics.Counter

	Pe8PhvFfOvflow metrics.Counter

	Pe9PhvFfOvflow metrics.Counter

	Pe0OhiFfOvflow metrics.Counter

	Pe1OhiFfOvflow metrics.Counter

	Pe2OhiFfOvflow metrics.Counter

	Pe3OhiFfOvflow metrics.Counter

	Pe4OhiFfOvflow metrics.Counter

	Pe5OhiFfOvflow metrics.Counter

	Pe6OhiFfOvflow metrics.Counter

	Pe7OhiFfOvflow metrics.Counter

	Pe8OhiFfOvflow metrics.Counter

	Pe9OhiFfOvflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intfifo1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intfifo1Metrics) Size() int {
	sz := 0

	sz += mtr.Pe0QctlFfOvflow.Size()

	sz += mtr.Pe1QctlFfOvflow.Size()

	sz += mtr.Pe2QctlFfOvflow.Size()

	sz += mtr.Pe3QctlFfOvflow.Size()

	sz += mtr.Pe4QctlFfOvflow.Size()

	sz += mtr.Pe5QctlFfOvflow.Size()

	sz += mtr.Pe6QctlFfOvflow.Size()

	sz += mtr.Pe7QctlFfOvflow.Size()

	sz += mtr.Pe8QctlFfOvflow.Size()

	sz += mtr.Pe9QctlFfOvflow.Size()

	sz += mtr.Pe0PhvFfOvflow.Size()

	sz += mtr.Pe1PhvFfOvflow.Size()

	sz += mtr.Pe2PhvFfOvflow.Size()

	sz += mtr.Pe3PhvFfOvflow.Size()

	sz += mtr.Pe4PhvFfOvflow.Size()

	sz += mtr.Pe5PhvFfOvflow.Size()

	sz += mtr.Pe6PhvFfOvflow.Size()

	sz += mtr.Pe7PhvFfOvflow.Size()

	sz += mtr.Pe8PhvFfOvflow.Size()

	sz += mtr.Pe9PhvFfOvflow.Size()

	sz += mtr.Pe0OhiFfOvflow.Size()

	sz += mtr.Pe1OhiFfOvflow.Size()

	sz += mtr.Pe2OhiFfOvflow.Size()

	sz += mtr.Pe3OhiFfOvflow.Size()

	sz += mtr.Pe4OhiFfOvflow.Size()

	sz += mtr.Pe5OhiFfOvflow.Size()

	sz += mtr.Pe6OhiFfOvflow.Size()

	sz += mtr.Pe7OhiFfOvflow.Size()

	sz += mtr.Pe8OhiFfOvflow.Size()

	sz += mtr.Pe9OhiFfOvflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intfifo1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Pe0QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe0QctlFfOvflow.Size()

	mtr.Pe1QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe1QctlFfOvflow.Size()

	mtr.Pe2QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe2QctlFfOvflow.Size()

	mtr.Pe3QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe3QctlFfOvflow.Size()

	mtr.Pe4QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe4QctlFfOvflow.Size()

	mtr.Pe5QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe5QctlFfOvflow.Size()

	mtr.Pe6QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe6QctlFfOvflow.Size()

	mtr.Pe7QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe7QctlFfOvflow.Size()

	mtr.Pe8QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe8QctlFfOvflow.Size()

	mtr.Pe9QctlFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe9QctlFfOvflow.Size()

	mtr.Pe0PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe0PhvFfOvflow.Size()

	mtr.Pe1PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe1PhvFfOvflow.Size()

	mtr.Pe2PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe2PhvFfOvflow.Size()

	mtr.Pe3PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe3PhvFfOvflow.Size()

	mtr.Pe4PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe4PhvFfOvflow.Size()

	mtr.Pe5PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe5PhvFfOvflow.Size()

	mtr.Pe6PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe6PhvFfOvflow.Size()

	mtr.Pe7PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe7PhvFfOvflow.Size()

	mtr.Pe8PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe8PhvFfOvflow.Size()

	mtr.Pe9PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe9PhvFfOvflow.Size()

	mtr.Pe0OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe0OhiFfOvflow.Size()

	mtr.Pe1OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe1OhiFfOvflow.Size()

	mtr.Pe2OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe2OhiFfOvflow.Size()

	mtr.Pe3OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe3OhiFfOvflow.Size()

	mtr.Pe4OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe4OhiFfOvflow.Size()

	mtr.Pe5OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe5OhiFfOvflow.Size()

	mtr.Pe6OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe6OhiFfOvflow.Size()

	mtr.Pe7OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe7OhiFfOvflow.Size()

	mtr.Pe8OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe8OhiFfOvflow.Size()

	mtr.Pe9OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.Pe9OhiFfOvflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intfifo1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Pe0QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe0QctlFfOvflow.Size()

	if fldName == "Pe1QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe1QctlFfOvflow.Size()

	if fldName == "Pe2QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe2QctlFfOvflow.Size()

	if fldName == "Pe3QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe3QctlFfOvflow.Size()

	if fldName == "Pe4QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe4QctlFfOvflow.Size()

	if fldName == "Pe5QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe5QctlFfOvflow.Size()

	if fldName == "Pe6QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe6QctlFfOvflow.Size()

	if fldName == "Pe7QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe7QctlFfOvflow.Size()

	if fldName == "Pe8QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe8QctlFfOvflow.Size()

	if fldName == "Pe9QctlFfOvflow" {
		return offset
	}
	offset += mtr.Pe9QctlFfOvflow.Size()

	if fldName == "Pe0PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe0PhvFfOvflow.Size()

	if fldName == "Pe1PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe1PhvFfOvflow.Size()

	if fldName == "Pe2PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe2PhvFfOvflow.Size()

	if fldName == "Pe3PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe3PhvFfOvflow.Size()

	if fldName == "Pe4PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe4PhvFfOvflow.Size()

	if fldName == "Pe5PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe5PhvFfOvflow.Size()

	if fldName == "Pe6PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe6PhvFfOvflow.Size()

	if fldName == "Pe7PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe7PhvFfOvflow.Size()

	if fldName == "Pe8PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe8PhvFfOvflow.Size()

	if fldName == "Pe9PhvFfOvflow" {
		return offset
	}
	offset += mtr.Pe9PhvFfOvflow.Size()

	if fldName == "Pe0OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe0OhiFfOvflow.Size()

	if fldName == "Pe1OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe1OhiFfOvflow.Size()

	if fldName == "Pe2OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe2OhiFfOvflow.Size()

	if fldName == "Pe3OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe3OhiFfOvflow.Size()

	if fldName == "Pe4OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe4OhiFfOvflow.Size()

	if fldName == "Pe5OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe5OhiFfOvflow.Size()

	if fldName == "Pe6OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe6OhiFfOvflow.Size()

	if fldName == "Pe7OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe7OhiFfOvflow.Size()

	if fldName == "Pe8OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe8OhiFfOvflow.Size()

	if fldName == "Pe9OhiFfOvflow" {
		return offset
	}
	offset += mtr.Pe9OhiFfOvflow.Size()

	return offset
}

// SetPe0QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe0QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe0QctlFfOvflow"))
	return nil
}

// SetPe1QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe1QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe1QctlFfOvflow"))
	return nil
}

// SetPe2QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe2QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe2QctlFfOvflow"))
	return nil
}

// SetPe3QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe3QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe3QctlFfOvflow"))
	return nil
}

// SetPe4QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe4QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe4QctlFfOvflow"))
	return nil
}

// SetPe5QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe5QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe5QctlFfOvflow"))
	return nil
}

// SetPe6QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe6QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe6QctlFfOvflow"))
	return nil
}

// SetPe7QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe7QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe7QctlFfOvflow"))
	return nil
}

// SetPe8QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe8QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe8QctlFfOvflow"))
	return nil
}

// SetPe9QctlFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe9QctlFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe9QctlFfOvflow"))
	return nil
}

// SetPe0PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe0PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe0PhvFfOvflow"))
	return nil
}

// SetPe1PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe1PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe1PhvFfOvflow"))
	return nil
}

// SetPe2PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe2PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe2PhvFfOvflow"))
	return nil
}

// SetPe3PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe3PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe3PhvFfOvflow"))
	return nil
}

// SetPe4PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe4PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe4PhvFfOvflow"))
	return nil
}

// SetPe5PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe5PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe5PhvFfOvflow"))
	return nil
}

// SetPe6PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe6PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe6PhvFfOvflow"))
	return nil
}

// SetPe7PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe7PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe7PhvFfOvflow"))
	return nil
}

// SetPe8PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe8PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe8PhvFfOvflow"))
	return nil
}

// SetPe9PhvFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe9PhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe9PhvFfOvflow"))
	return nil
}

// SetPe0OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe0OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe0OhiFfOvflow"))
	return nil
}

// SetPe1OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe1OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe1OhiFfOvflow"))
	return nil
}

// SetPe2OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe2OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe2OhiFfOvflow"))
	return nil
}

// SetPe3OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe3OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe3OhiFfOvflow"))
	return nil
}

// SetPe4OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe4OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe4OhiFfOvflow"))
	return nil
}

// SetPe5OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe5OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe5OhiFfOvflow"))
	return nil
}

// SetPe6OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe6OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe6OhiFfOvflow"))
	return nil
}

// SetPe7OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe7OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe7OhiFfOvflow"))
	return nil
}

// SetPe8OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe8OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe8OhiFfOvflow"))
	return nil
}

// SetPe9OhiFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo1Metrics) SetPe9OhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pe9OhiFfOvflow"))
	return nil
}

// Ppappa1intfifo1MetricsIterator is the iterator object
type Ppappa1intfifo1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intfifo1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intfifo1MetricsIterator) Next() *Ppappa1intfifo1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intfifo1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intfifo1MetricsIterator) Find(key uint64) (*Ppappa1intfifo1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intfifo1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intfifo1MetricsIterator) Create(key uint64) (*Ppappa1intfifo1Metrics, error) {
	tmtr := &Ppappa1intfifo1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intfifo1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intfifo1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intfifo1MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intfifo1MetricsIterator returns an iterator
func NewPpappa1intfifo1MetricsIterator() (*Ppappa1intfifo1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intfifo1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intfifo1MetricsIterator{iter: iter}, nil
}

type Ppappa1intfifo2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DpIfFfOvflow metrics.Counter

	OhiIfFfOvflow metrics.Counter

	MaIfFfOvflow metrics.Counter

	PbIfFfOvflow metrics.Counter

	PktMtuFfOvflow metrics.Counter

	OutphvFfOvflow metrics.Counter

	OutohiFfOvflow metrics.Counter

	ChkFfOvflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intfifo2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intfifo2Metrics) Size() int {
	sz := 0

	sz += mtr.DpIfFfOvflow.Size()

	sz += mtr.OhiIfFfOvflow.Size()

	sz += mtr.MaIfFfOvflow.Size()

	sz += mtr.PbIfFfOvflow.Size()

	sz += mtr.PktMtuFfOvflow.Size()

	sz += mtr.OutphvFfOvflow.Size()

	sz += mtr.OutohiFfOvflow.Size()

	sz += mtr.ChkFfOvflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intfifo2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.DpIfFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DpIfFfOvflow.Size()

	mtr.OhiIfFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.OhiIfFfOvflow.Size()

	mtr.MaIfFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.MaIfFfOvflow.Size()

	mtr.PbIfFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PbIfFfOvflow.Size()

	mtr.PktMtuFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktMtuFfOvflow.Size()

	mtr.OutphvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.OutphvFfOvflow.Size()

	mtr.OutohiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.OutohiFfOvflow.Size()

	mtr.ChkFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ChkFfOvflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intfifo2Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "DpIfFfOvflow" {
		return offset
	}
	offset += mtr.DpIfFfOvflow.Size()

	if fldName == "OhiIfFfOvflow" {
		return offset
	}
	offset += mtr.OhiIfFfOvflow.Size()

	if fldName == "MaIfFfOvflow" {
		return offset
	}
	offset += mtr.MaIfFfOvflow.Size()

	if fldName == "PbIfFfOvflow" {
		return offset
	}
	offset += mtr.PbIfFfOvflow.Size()

	if fldName == "PktMtuFfOvflow" {
		return offset
	}
	offset += mtr.PktMtuFfOvflow.Size()

	if fldName == "OutphvFfOvflow" {
		return offset
	}
	offset += mtr.OutphvFfOvflow.Size()

	if fldName == "OutohiFfOvflow" {
		return offset
	}
	offset += mtr.OutohiFfOvflow.Size()

	if fldName == "ChkFfOvflow" {
		return offset
	}
	offset += mtr.ChkFfOvflow.Size()

	return offset
}

// SetDpIfFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo2Metrics) SetDpIfFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DpIfFfOvflow"))
	return nil
}

// SetOhiIfFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo2Metrics) SetOhiIfFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiIfFfOvflow"))
	return nil
}

// SetMaIfFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo2Metrics) SetMaIfFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaIfFfOvflow"))
	return nil
}

// SetPbIfFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo2Metrics) SetPbIfFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbIfFfOvflow"))
	return nil
}

// SetPktMtuFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo2Metrics) SetPktMtuFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktMtuFfOvflow"))
	return nil
}

// SetOutphvFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo2Metrics) SetOutphvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OutphvFfOvflow"))
	return nil
}

// SetOutohiFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo2Metrics) SetOutohiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OutohiFfOvflow"))
	return nil
}

// SetChkFfOvflow sets cunter in shared memory
func (mtr *Ppappa1intfifo2Metrics) SetChkFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ChkFfOvflow"))
	return nil
}

// Ppappa1intfifo2MetricsIterator is the iterator object
type Ppappa1intfifo2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intfifo2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intfifo2MetricsIterator) Next() *Ppappa1intfifo2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intfifo2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intfifo2MetricsIterator) Find(key uint64) (*Ppappa1intfifo2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intfifo2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intfifo2MetricsIterator) Create(key uint64) (*Ppappa1intfifo2Metrics, error) {
	tmtr := &Ppappa1intfifo2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intfifo2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intfifo2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intfifo2MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intfifo2MetricsIterator returns an iterator
func NewPpappa1intfifo2MetricsIterator() (*Ppappa1intfifo2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intfifo2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intfifo2MetricsIterator{iter: iter}, nil
}

type Ppappa1intpe0Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intpe0Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intpe0Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intpe0Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intpe0Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa1intpe0Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa1intpe0Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa1intpe0Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa1intpe0Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa1intpe0Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa1intpe0Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa1intpe0Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa1intpe0Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa1intpe0Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa1intpe0Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa1intpe0MetricsIterator is the iterator object
type Ppappa1intpe0MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intpe0MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intpe0MetricsIterator) Next() *Ppappa1intpe0Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intpe0Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intpe0MetricsIterator) Find(key uint64) (*Ppappa1intpe0Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intpe0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intpe0MetricsIterator) Create(key uint64) (*Ppappa1intpe0Metrics, error) {
	tmtr := &Ppappa1intpe0Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intpe0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intpe0MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intpe0MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intpe0MetricsIterator returns an iterator
func NewPpappa1intpe0MetricsIterator() (*Ppappa1intpe0MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intpe0Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intpe0MetricsIterator{iter: iter}, nil
}

type Ppappa1intpe1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intpe1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intpe1Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intpe1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intpe1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa1intpe1Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa1intpe1Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa1intpe1Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa1intpe1Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa1intpe1Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa1intpe1Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa1intpe1Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa1intpe1Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa1intpe1Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa1intpe1Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa1intpe1MetricsIterator is the iterator object
type Ppappa1intpe1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intpe1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intpe1MetricsIterator) Next() *Ppappa1intpe1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intpe1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intpe1MetricsIterator) Find(key uint64) (*Ppappa1intpe1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intpe1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intpe1MetricsIterator) Create(key uint64) (*Ppappa1intpe1Metrics, error) {
	tmtr := &Ppappa1intpe1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intpe1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intpe1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intpe1MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intpe1MetricsIterator returns an iterator
func NewPpappa1intpe1MetricsIterator() (*Ppappa1intpe1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intpe1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intpe1MetricsIterator{iter: iter}, nil
}

type Ppappa1intpe2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intpe2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intpe2Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intpe2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intpe2Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa1intpe2Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa1intpe2Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa1intpe2Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa1intpe2Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa1intpe2Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa1intpe2Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa1intpe2Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa1intpe2Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa1intpe2Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa1intpe2Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa1intpe2MetricsIterator is the iterator object
type Ppappa1intpe2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intpe2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intpe2MetricsIterator) Next() *Ppappa1intpe2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intpe2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intpe2MetricsIterator) Find(key uint64) (*Ppappa1intpe2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intpe2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intpe2MetricsIterator) Create(key uint64) (*Ppappa1intpe2Metrics, error) {
	tmtr := &Ppappa1intpe2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intpe2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intpe2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intpe2MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intpe2MetricsIterator returns an iterator
func NewPpappa1intpe2MetricsIterator() (*Ppappa1intpe2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intpe2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intpe2MetricsIterator{iter: iter}, nil
}

type Ppappa1intpe3Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intpe3Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intpe3Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intpe3Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intpe3Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa1intpe3Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa1intpe3Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa1intpe3Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa1intpe3Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa1intpe3Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa1intpe3Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa1intpe3Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa1intpe3Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa1intpe3Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa1intpe3Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa1intpe3MetricsIterator is the iterator object
type Ppappa1intpe3MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intpe3MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intpe3MetricsIterator) Next() *Ppappa1intpe3Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intpe3Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intpe3MetricsIterator) Find(key uint64) (*Ppappa1intpe3Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intpe3Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intpe3MetricsIterator) Create(key uint64) (*Ppappa1intpe3Metrics, error) {
	tmtr := &Ppappa1intpe3Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intpe3Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intpe3MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intpe3MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intpe3MetricsIterator returns an iterator
func NewPpappa1intpe3MetricsIterator() (*Ppappa1intpe3MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intpe3Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intpe3MetricsIterator{iter: iter}, nil
}

type Ppappa1intpe4Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intpe4Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intpe4Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intpe4Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intpe4Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa1intpe4Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa1intpe4Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa1intpe4Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa1intpe4Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa1intpe4Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa1intpe4Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa1intpe4Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa1intpe4Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa1intpe4Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa1intpe4Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa1intpe4MetricsIterator is the iterator object
type Ppappa1intpe4MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intpe4MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intpe4MetricsIterator) Next() *Ppappa1intpe4Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intpe4Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intpe4MetricsIterator) Find(key uint64) (*Ppappa1intpe4Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intpe4Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intpe4MetricsIterator) Create(key uint64) (*Ppappa1intpe4Metrics, error) {
	tmtr := &Ppappa1intpe4Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intpe4Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intpe4MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intpe4MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intpe4MetricsIterator returns an iterator
func NewPpappa1intpe4MetricsIterator() (*Ppappa1intpe4MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intpe4Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intpe4MetricsIterator{iter: iter}, nil
}

type Ppappa1intpe5Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intpe5Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intpe5Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intpe5Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intpe5Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa1intpe5Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa1intpe5Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa1intpe5Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa1intpe5Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa1intpe5Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa1intpe5Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa1intpe5Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa1intpe5Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa1intpe5Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa1intpe5Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa1intpe5MetricsIterator is the iterator object
type Ppappa1intpe5MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intpe5MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intpe5MetricsIterator) Next() *Ppappa1intpe5Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intpe5Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intpe5MetricsIterator) Find(key uint64) (*Ppappa1intpe5Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intpe5Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intpe5MetricsIterator) Create(key uint64) (*Ppappa1intpe5Metrics, error) {
	tmtr := &Ppappa1intpe5Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intpe5Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intpe5MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intpe5MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intpe5MetricsIterator returns an iterator
func NewPpappa1intpe5MetricsIterator() (*Ppappa1intpe5MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intpe5Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intpe5MetricsIterator{iter: iter}, nil
}

type Ppappa1intpe6Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intpe6Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intpe6Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intpe6Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intpe6Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa1intpe6Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa1intpe6Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa1intpe6Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa1intpe6Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa1intpe6Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa1intpe6Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa1intpe6Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa1intpe6Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa1intpe6Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa1intpe6Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa1intpe6MetricsIterator is the iterator object
type Ppappa1intpe6MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intpe6MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intpe6MetricsIterator) Next() *Ppappa1intpe6Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intpe6Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intpe6MetricsIterator) Find(key uint64) (*Ppappa1intpe6Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intpe6Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intpe6MetricsIterator) Create(key uint64) (*Ppappa1intpe6Metrics, error) {
	tmtr := &Ppappa1intpe6Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intpe6Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intpe6MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intpe6MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intpe6MetricsIterator returns an iterator
func NewPpappa1intpe6MetricsIterator() (*Ppappa1intpe6MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intpe6Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intpe6MetricsIterator{iter: iter}, nil
}

type Ppappa1intpe7Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intpe7Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intpe7Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intpe7Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intpe7Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa1intpe7Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa1intpe7Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa1intpe7Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa1intpe7Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa1intpe7Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa1intpe7Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa1intpe7Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa1intpe7Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa1intpe7Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa1intpe7Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa1intpe7MetricsIterator is the iterator object
type Ppappa1intpe7MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intpe7MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intpe7MetricsIterator) Next() *Ppappa1intpe7Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intpe7Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intpe7MetricsIterator) Find(key uint64) (*Ppappa1intpe7Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intpe7Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intpe7MetricsIterator) Create(key uint64) (*Ppappa1intpe7Metrics, error) {
	tmtr := &Ppappa1intpe7Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intpe7Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intpe7MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intpe7MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intpe7MetricsIterator returns an iterator
func NewPpappa1intpe7MetricsIterator() (*Ppappa1intpe7MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intpe7Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intpe7MetricsIterator{iter: iter}, nil
}

type Ppappa1intpe8Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intpe8Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intpe8Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intpe8Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intpe8Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa1intpe8Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa1intpe8Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa1intpe8Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa1intpe8Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa1intpe8Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa1intpe8Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa1intpe8Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa1intpe8Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa1intpe8Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa1intpe8Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa1intpe8MetricsIterator is the iterator object
type Ppappa1intpe8MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intpe8MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intpe8MetricsIterator) Next() *Ppappa1intpe8Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intpe8Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intpe8MetricsIterator) Find(key uint64) (*Ppappa1intpe8Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intpe8Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intpe8MetricsIterator) Create(key uint64) (*Ppappa1intpe8Metrics, error) {
	tmtr := &Ppappa1intpe8Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intpe8Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intpe8MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intpe8MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intpe8MetricsIterator returns an iterator
func NewPpappa1intpe8MetricsIterator() (*Ppappa1intpe8MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intpe8Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intpe8MetricsIterator{iter: iter}, nil
}

type Ppappa1intpe9Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QctlSkidDepthNotZero metrics.Counter

	QctlNotSopAfterPktRelease metrics.Counter

	UnexpectedNonSop metrics.Counter

	ExceedPhvFlitCnt metrics.Counter

	ExceedParseLoopCnt metrics.Counter

	OffsetOutOfRange metrics.Counter

	PktSizeChk metrics.Counter

	OffsetJumpChk metrics.Counter

	D1NotValid metrics.Counter

	PhvUprIdxLess metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intpe9Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intpe9Metrics) Size() int {
	sz := 0

	sz += mtr.QctlSkidDepthNotZero.Size()

	sz += mtr.QctlNotSopAfterPktRelease.Size()

	sz += mtr.UnexpectedNonSop.Size()

	sz += mtr.ExceedPhvFlitCnt.Size()

	sz += mtr.ExceedParseLoopCnt.Size()

	sz += mtr.OffsetOutOfRange.Size()

	sz += mtr.PktSizeChk.Size()

	sz += mtr.OffsetJumpChk.Size()

	sz += mtr.D1NotValid.Size()

	sz += mtr.PhvUprIdxLess.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intpe9Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QctlSkidDepthNotZero = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlSkidDepthNotZero.Size()

	mtr.QctlNotSopAfterPktRelease = mtr.metrics.GetCounter(offset)
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	mtr.UnexpectedNonSop = mtr.metrics.GetCounter(offset)
	offset += mtr.UnexpectedNonSop.Size()

	mtr.ExceedPhvFlitCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedPhvFlitCnt.Size()

	mtr.ExceedParseLoopCnt = mtr.metrics.GetCounter(offset)
	offset += mtr.ExceedParseLoopCnt.Size()

	mtr.OffsetOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetOutOfRange.Size()

	mtr.PktSizeChk = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeChk.Size()

	mtr.OffsetJumpChk = mtr.metrics.GetCounter(offset)
	offset += mtr.OffsetJumpChk.Size()

	mtr.D1NotValid = mtr.metrics.GetCounter(offset)
	offset += mtr.D1NotValid.Size()

	mtr.PhvUprIdxLess = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvUprIdxLess.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intpe9Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QctlSkidDepthNotZero" {
		return offset
	}
	offset += mtr.QctlSkidDepthNotZero.Size()

	if fldName == "QctlNotSopAfterPktRelease" {
		return offset
	}
	offset += mtr.QctlNotSopAfterPktRelease.Size()

	if fldName == "UnexpectedNonSop" {
		return offset
	}
	offset += mtr.UnexpectedNonSop.Size()

	if fldName == "ExceedPhvFlitCnt" {
		return offset
	}
	offset += mtr.ExceedPhvFlitCnt.Size()

	if fldName == "ExceedParseLoopCnt" {
		return offset
	}
	offset += mtr.ExceedParseLoopCnt.Size()

	if fldName == "OffsetOutOfRange" {
		return offset
	}
	offset += mtr.OffsetOutOfRange.Size()

	if fldName == "PktSizeChk" {
		return offset
	}
	offset += mtr.PktSizeChk.Size()

	if fldName == "OffsetJumpChk" {
		return offset
	}
	offset += mtr.OffsetJumpChk.Size()

	if fldName == "D1NotValid" {
		return offset
	}
	offset += mtr.D1NotValid.Size()

	if fldName == "PhvUprIdxLess" {
		return offset
	}
	offset += mtr.PhvUprIdxLess.Size()

	return offset
}

// SetQctlSkidDepthNotZero sets cunter in shared memory
func (mtr *Ppappa1intpe9Metrics) SetQctlSkidDepthNotZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlSkidDepthNotZero"))
	return nil
}

// SetQctlNotSopAfterPktRelease sets cunter in shared memory
func (mtr *Ppappa1intpe9Metrics) SetQctlNotSopAfterPktRelease(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QctlNotSopAfterPktRelease"))
	return nil
}

// SetUnexpectedNonSop sets cunter in shared memory
func (mtr *Ppappa1intpe9Metrics) SetUnexpectedNonSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnexpectedNonSop"))
	return nil
}

// SetExceedPhvFlitCnt sets cunter in shared memory
func (mtr *Ppappa1intpe9Metrics) SetExceedPhvFlitCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedPhvFlitCnt"))
	return nil
}

// SetExceedParseLoopCnt sets cunter in shared memory
func (mtr *Ppappa1intpe9Metrics) SetExceedParseLoopCnt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ExceedParseLoopCnt"))
	return nil
}

// SetOffsetOutOfRange sets cunter in shared memory
func (mtr *Ppappa1intpe9Metrics) SetOffsetOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetOutOfRange"))
	return nil
}

// SetPktSizeChk sets cunter in shared memory
func (mtr *Ppappa1intpe9Metrics) SetPktSizeChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeChk"))
	return nil
}

// SetOffsetJumpChk sets cunter in shared memory
func (mtr *Ppappa1intpe9Metrics) SetOffsetJumpChk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OffsetJumpChk"))
	return nil
}

// SetD1NotValid sets cunter in shared memory
func (mtr *Ppappa1intpe9Metrics) SetD1NotValid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("D1NotValid"))
	return nil
}

// SetPhvUprIdxLess sets cunter in shared memory
func (mtr *Ppappa1intpe9Metrics) SetPhvUprIdxLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvUprIdxLess"))
	return nil
}

// Ppappa1intpe9MetricsIterator is the iterator object
type Ppappa1intpe9MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intpe9MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intpe9MetricsIterator) Next() *Ppappa1intpe9Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intpe9Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intpe9MetricsIterator) Find(key uint64) (*Ppappa1intpe9Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intpe9Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intpe9MetricsIterator) Create(key uint64) (*Ppappa1intpe9Metrics, error) {
	tmtr := &Ppappa1intpe9Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intpe9Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intpe9MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intpe9MetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intpe9MetricsIterator returns an iterator
func NewPpappa1intpe9MetricsIterator() (*Ppappa1intpe9MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intpe9Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intpe9MetricsIterator{iter: iter}, nil
}

type Ppappa1intpaMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	ReorderPhvNotSop metrics.Counter

	Csum0NotEnufBytes metrics.Counter

	Csum0StartMoreThanEndOffset metrics.Counter

	Csum1NotEnufBytes metrics.Counter

	Csum1StartMoreThanEndOffset metrics.Counter

	Csum2NotEnufBytes metrics.Counter

	Csum2StartMoreThanEndOffset metrics.Counter

	Csum3NotEnufBytes metrics.Counter

	Csum3StartMoreThanEndOffset metrics.Counter

	Csum4NotEnufBytes metrics.Counter

	Csum4StartMoreThanEndOffset metrics.Counter

	CrcNotEnufBytes metrics.Counter

	CrcStartMoreThanEndOffset metrics.Counter

	ChkrPhvSingleFlit metrics.Counter

	ChkrPhvNotSop metrics.Counter

	ChkrPktNotSop metrics.Counter

	ChkrSeqId metrics.Counter

	PreparseLine0NotSop metrics.Counter

	PreparseDffNotSop metrics.Counter

	PreparseDffOvflow metrics.Counter

	PreparseEffOvflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intpaMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intpaMetrics) Size() int {
	sz := 0

	sz += mtr.ReorderPhvNotSop.Size()

	sz += mtr.Csum0NotEnufBytes.Size()

	sz += mtr.Csum0StartMoreThanEndOffset.Size()

	sz += mtr.Csum1NotEnufBytes.Size()

	sz += mtr.Csum1StartMoreThanEndOffset.Size()

	sz += mtr.Csum2NotEnufBytes.Size()

	sz += mtr.Csum2StartMoreThanEndOffset.Size()

	sz += mtr.Csum3NotEnufBytes.Size()

	sz += mtr.Csum3StartMoreThanEndOffset.Size()

	sz += mtr.Csum4NotEnufBytes.Size()

	sz += mtr.Csum4StartMoreThanEndOffset.Size()

	sz += mtr.CrcNotEnufBytes.Size()

	sz += mtr.CrcStartMoreThanEndOffset.Size()

	sz += mtr.ChkrPhvSingleFlit.Size()

	sz += mtr.ChkrPhvNotSop.Size()

	sz += mtr.ChkrPktNotSop.Size()

	sz += mtr.ChkrSeqId.Size()

	sz += mtr.PreparseLine0NotSop.Size()

	sz += mtr.PreparseDffNotSop.Size()

	sz += mtr.PreparseDffOvflow.Size()

	sz += mtr.PreparseEffOvflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intpaMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.ReorderPhvNotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.ReorderPhvNotSop.Size()

	mtr.Csum0NotEnufBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum0NotEnufBytes.Size()

	mtr.Csum0StartMoreThanEndOffset = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum0StartMoreThanEndOffset.Size()

	mtr.Csum1NotEnufBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum1NotEnufBytes.Size()

	mtr.Csum1StartMoreThanEndOffset = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum1StartMoreThanEndOffset.Size()

	mtr.Csum2NotEnufBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum2NotEnufBytes.Size()

	mtr.Csum2StartMoreThanEndOffset = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum2StartMoreThanEndOffset.Size()

	mtr.Csum3NotEnufBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum3NotEnufBytes.Size()

	mtr.Csum3StartMoreThanEndOffset = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum3StartMoreThanEndOffset.Size()

	mtr.Csum4NotEnufBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum4NotEnufBytes.Size()

	mtr.Csum4StartMoreThanEndOffset = mtr.metrics.GetCounter(offset)
	offset += mtr.Csum4StartMoreThanEndOffset.Size()

	mtr.CrcNotEnufBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.CrcNotEnufBytes.Size()

	mtr.CrcStartMoreThanEndOffset = mtr.metrics.GetCounter(offset)
	offset += mtr.CrcStartMoreThanEndOffset.Size()

	mtr.ChkrPhvSingleFlit = mtr.metrics.GetCounter(offset)
	offset += mtr.ChkrPhvSingleFlit.Size()

	mtr.ChkrPhvNotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.ChkrPhvNotSop.Size()

	mtr.ChkrPktNotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.ChkrPktNotSop.Size()

	mtr.ChkrSeqId = mtr.metrics.GetCounter(offset)
	offset += mtr.ChkrSeqId.Size()

	mtr.PreparseLine0NotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.PreparseLine0NotSop.Size()

	mtr.PreparseDffNotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.PreparseDffNotSop.Size()

	mtr.PreparseDffOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PreparseDffOvflow.Size()

	mtr.PreparseEffOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PreparseEffOvflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intpaMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "ReorderPhvNotSop" {
		return offset
	}
	offset += mtr.ReorderPhvNotSop.Size()

	if fldName == "Csum0NotEnufBytes" {
		return offset
	}
	offset += mtr.Csum0NotEnufBytes.Size()

	if fldName == "Csum0StartMoreThanEndOffset" {
		return offset
	}
	offset += mtr.Csum0StartMoreThanEndOffset.Size()

	if fldName == "Csum1NotEnufBytes" {
		return offset
	}
	offset += mtr.Csum1NotEnufBytes.Size()

	if fldName == "Csum1StartMoreThanEndOffset" {
		return offset
	}
	offset += mtr.Csum1StartMoreThanEndOffset.Size()

	if fldName == "Csum2NotEnufBytes" {
		return offset
	}
	offset += mtr.Csum2NotEnufBytes.Size()

	if fldName == "Csum2StartMoreThanEndOffset" {
		return offset
	}
	offset += mtr.Csum2StartMoreThanEndOffset.Size()

	if fldName == "Csum3NotEnufBytes" {
		return offset
	}
	offset += mtr.Csum3NotEnufBytes.Size()

	if fldName == "Csum3StartMoreThanEndOffset" {
		return offset
	}
	offset += mtr.Csum3StartMoreThanEndOffset.Size()

	if fldName == "Csum4NotEnufBytes" {
		return offset
	}
	offset += mtr.Csum4NotEnufBytes.Size()

	if fldName == "Csum4StartMoreThanEndOffset" {
		return offset
	}
	offset += mtr.Csum4StartMoreThanEndOffset.Size()

	if fldName == "CrcNotEnufBytes" {
		return offset
	}
	offset += mtr.CrcNotEnufBytes.Size()

	if fldName == "CrcStartMoreThanEndOffset" {
		return offset
	}
	offset += mtr.CrcStartMoreThanEndOffset.Size()

	if fldName == "ChkrPhvSingleFlit" {
		return offset
	}
	offset += mtr.ChkrPhvSingleFlit.Size()

	if fldName == "ChkrPhvNotSop" {
		return offset
	}
	offset += mtr.ChkrPhvNotSop.Size()

	if fldName == "ChkrPktNotSop" {
		return offset
	}
	offset += mtr.ChkrPktNotSop.Size()

	if fldName == "ChkrSeqId" {
		return offset
	}
	offset += mtr.ChkrSeqId.Size()

	if fldName == "PreparseLine0NotSop" {
		return offset
	}
	offset += mtr.PreparseLine0NotSop.Size()

	if fldName == "PreparseDffNotSop" {
		return offset
	}
	offset += mtr.PreparseDffNotSop.Size()

	if fldName == "PreparseDffOvflow" {
		return offset
	}
	offset += mtr.PreparseDffOvflow.Size()

	if fldName == "PreparseEffOvflow" {
		return offset
	}
	offset += mtr.PreparseEffOvflow.Size()

	return offset
}

// SetReorderPhvNotSop sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetReorderPhvNotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ReorderPhvNotSop"))
	return nil
}

// SetCsum0NotEnufBytes sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetCsum0NotEnufBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum0NotEnufBytes"))
	return nil
}

// SetCsum0StartMoreThanEndOffset sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetCsum0StartMoreThanEndOffset(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum0StartMoreThanEndOffset"))
	return nil
}

// SetCsum1NotEnufBytes sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetCsum1NotEnufBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum1NotEnufBytes"))
	return nil
}

// SetCsum1StartMoreThanEndOffset sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetCsum1StartMoreThanEndOffset(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum1StartMoreThanEndOffset"))
	return nil
}

// SetCsum2NotEnufBytes sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetCsum2NotEnufBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum2NotEnufBytes"))
	return nil
}

// SetCsum2StartMoreThanEndOffset sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetCsum2StartMoreThanEndOffset(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum2StartMoreThanEndOffset"))
	return nil
}

// SetCsum3NotEnufBytes sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetCsum3NotEnufBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum3NotEnufBytes"))
	return nil
}

// SetCsum3StartMoreThanEndOffset sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetCsum3StartMoreThanEndOffset(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum3StartMoreThanEndOffset"))
	return nil
}

// SetCsum4NotEnufBytes sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetCsum4NotEnufBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum4NotEnufBytes"))
	return nil
}

// SetCsum4StartMoreThanEndOffset sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetCsum4StartMoreThanEndOffset(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Csum4StartMoreThanEndOffset"))
	return nil
}

// SetCrcNotEnufBytes sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetCrcNotEnufBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CrcNotEnufBytes"))
	return nil
}

// SetCrcStartMoreThanEndOffset sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetCrcStartMoreThanEndOffset(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CrcStartMoreThanEndOffset"))
	return nil
}

// SetChkrPhvSingleFlit sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetChkrPhvSingleFlit(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ChkrPhvSingleFlit"))
	return nil
}

// SetChkrPhvNotSop sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetChkrPhvNotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ChkrPhvNotSop"))
	return nil
}

// SetChkrPktNotSop sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetChkrPktNotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ChkrPktNotSop"))
	return nil
}

// SetChkrSeqId sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetChkrSeqId(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ChkrSeqId"))
	return nil
}

// SetPreparseLine0NotSop sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetPreparseLine0NotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PreparseLine0NotSop"))
	return nil
}

// SetPreparseDffNotSop sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetPreparseDffNotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PreparseDffNotSop"))
	return nil
}

// SetPreparseDffOvflow sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetPreparseDffOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PreparseDffOvflow"))
	return nil
}

// SetPreparseEffOvflow sets cunter in shared memory
func (mtr *Ppappa1intpaMetrics) SetPreparseEffOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PreparseEffOvflow"))
	return nil
}

// Ppappa1intpaMetricsIterator is the iterator object
type Ppappa1intpaMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intpaMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intpaMetricsIterator) Next() *Ppappa1intpaMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intpaMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intpaMetricsIterator) Find(key uint64) (*Ppappa1intpaMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intpaMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intpaMetricsIterator) Create(key uint64) (*Ppappa1intpaMetrics, error) {
	tmtr := &Ppappa1intpaMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intpaMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intpaMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intpaMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intpaMetricsIterator returns an iterator
func NewPpappa1intpaMetricsIterator() (*Ppappa1intpaMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intpaMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intpaMetricsIterator{iter: iter}, nil
}

type Ppappa1intswphvmemMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	EccUncorrectable metrics.Counter

	EccCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intswphvmemMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intswphvmemMetrics) Size() int {
	sz := 0

	sz += mtr.EccUncorrectable.Size()

	sz += mtr.EccCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intswphvmemMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.EccUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccUncorrectable.Size()

	mtr.EccCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intswphvmemMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "EccUncorrectable" {
		return offset
	}
	offset += mtr.EccUncorrectable.Size()

	if fldName == "EccCorrectable" {
		return offset
	}
	offset += mtr.EccCorrectable.Size()

	return offset
}

// SetEccUncorrectable sets cunter in shared memory
func (mtr *Ppappa1intswphvmemMetrics) SetEccUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccUncorrectable"))
	return nil
}

// SetEccCorrectable sets cunter in shared memory
func (mtr *Ppappa1intswphvmemMetrics) SetEccCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccCorrectable"))
	return nil
}

// Ppappa1intswphvmemMetricsIterator is the iterator object
type Ppappa1intswphvmemMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intswphvmemMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intswphvmemMetricsIterator) Next() *Ppappa1intswphvmemMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intswphvmemMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intswphvmemMetricsIterator) Find(key uint64) (*Ppappa1intswphvmemMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intswphvmemMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intswphvmemMetricsIterator) Create(key uint64) (*Ppappa1intswphvmemMetrics, error) {
	tmtr := &Ppappa1intswphvmemMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intswphvmemMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intswphvmemMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intswphvmemMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intswphvmemMetricsIterator returns an iterator
func NewPpappa1intswphvmemMetricsIterator() (*Ppappa1intswphvmemMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intswphvmemMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intswphvmemMetricsIterator{iter: iter}, nil
}

type Ppappa1intintfMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PbErr metrics.Counter

	PbSopErr metrics.Counter

	PbEopErr metrics.Counter

	DpErr metrics.Counter

	DpSopErr metrics.Counter

	DpEopErr metrics.Counter

	OhiErr metrics.Counter

	OhiSopErr metrics.Counter

	OhiEopErr metrics.Counter

	SwPhvErr metrics.Counter

	SwPhvSopErr metrics.Counter

	SwPhvEopErr metrics.Counter

	MaErr metrics.Counter

	MaSopErr metrics.Counter

	MaEopErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppappa1intintfMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppappa1intintfMetrics) Size() int {
	sz := 0

	sz += mtr.PbErr.Size()

	sz += mtr.PbSopErr.Size()

	sz += mtr.PbEopErr.Size()

	sz += mtr.DpErr.Size()

	sz += mtr.DpSopErr.Size()

	sz += mtr.DpEopErr.Size()

	sz += mtr.OhiErr.Size()

	sz += mtr.OhiSopErr.Size()

	sz += mtr.OhiEopErr.Size()

	sz += mtr.SwPhvErr.Size()

	sz += mtr.SwPhvSopErr.Size()

	sz += mtr.SwPhvEopErr.Size()

	sz += mtr.MaErr.Size()

	sz += mtr.MaSopErr.Size()

	sz += mtr.MaEopErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppappa1intintfMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PbErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbErr.Size()

	mtr.PbSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbSopErr.Size()

	mtr.PbEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbEopErr.Size()

	mtr.DpErr = mtr.metrics.GetCounter(offset)
	offset += mtr.DpErr.Size()

	mtr.DpSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.DpSopErr.Size()

	mtr.DpEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.DpEopErr.Size()

	mtr.OhiErr = mtr.metrics.GetCounter(offset)
	offset += mtr.OhiErr.Size()

	mtr.OhiSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.OhiSopErr.Size()

	mtr.OhiEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.OhiEopErr.Size()

	mtr.SwPhvErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SwPhvErr.Size()

	mtr.SwPhvSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SwPhvSopErr.Size()

	mtr.SwPhvEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SwPhvEopErr.Size()

	mtr.MaErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaErr.Size()

	mtr.MaSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaSopErr.Size()

	mtr.MaEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaEopErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppappa1intintfMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PbErr" {
		return offset
	}
	offset += mtr.PbErr.Size()

	if fldName == "PbSopErr" {
		return offset
	}
	offset += mtr.PbSopErr.Size()

	if fldName == "PbEopErr" {
		return offset
	}
	offset += mtr.PbEopErr.Size()

	if fldName == "DpErr" {
		return offset
	}
	offset += mtr.DpErr.Size()

	if fldName == "DpSopErr" {
		return offset
	}
	offset += mtr.DpSopErr.Size()

	if fldName == "DpEopErr" {
		return offset
	}
	offset += mtr.DpEopErr.Size()

	if fldName == "OhiErr" {
		return offset
	}
	offset += mtr.OhiErr.Size()

	if fldName == "OhiSopErr" {
		return offset
	}
	offset += mtr.OhiSopErr.Size()

	if fldName == "OhiEopErr" {
		return offset
	}
	offset += mtr.OhiEopErr.Size()

	if fldName == "SwPhvErr" {
		return offset
	}
	offset += mtr.SwPhvErr.Size()

	if fldName == "SwPhvSopErr" {
		return offset
	}
	offset += mtr.SwPhvSopErr.Size()

	if fldName == "SwPhvEopErr" {
		return offset
	}
	offset += mtr.SwPhvEopErr.Size()

	if fldName == "MaErr" {
		return offset
	}
	offset += mtr.MaErr.Size()

	if fldName == "MaSopErr" {
		return offset
	}
	offset += mtr.MaSopErr.Size()

	if fldName == "MaEopErr" {
		return offset
	}
	offset += mtr.MaEopErr.Size()

	return offset
}

// SetPbErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetPbErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbErr"))
	return nil
}

// SetPbSopErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetPbSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbSopErr"))
	return nil
}

// SetPbEopErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetPbEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbEopErr"))
	return nil
}

// SetDpErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetDpErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DpErr"))
	return nil
}

// SetDpSopErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetDpSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DpSopErr"))
	return nil
}

// SetDpEopErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetDpEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DpEopErr"))
	return nil
}

// SetOhiErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetOhiErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiErr"))
	return nil
}

// SetOhiSopErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetOhiSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiSopErr"))
	return nil
}

// SetOhiEopErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetOhiEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiEopErr"))
	return nil
}

// SetSwPhvErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetSwPhvErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SwPhvErr"))
	return nil
}

// SetSwPhvSopErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetSwPhvSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SwPhvSopErr"))
	return nil
}

// SetSwPhvEopErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetSwPhvEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SwPhvEopErr"))
	return nil
}

// SetMaErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetMaErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaErr"))
	return nil
}

// SetMaSopErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetMaSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaSopErr"))
	return nil
}

// SetMaEopErr sets cunter in shared memory
func (mtr *Ppappa1intintfMetrics) SetMaEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaEopErr"))
	return nil
}

// Ppappa1intintfMetricsIterator is the iterator object
type Ppappa1intintfMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppappa1intintfMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppappa1intintfMetricsIterator) Next() *Ppappa1intintfMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppappa1intintfMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppappa1intintfMetricsIterator) Find(key uint64) (*Ppappa1intintfMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppappa1intintfMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppappa1intintfMetricsIterator) Create(key uint64) (*Ppappa1intintfMetrics, error) {
	tmtr := &Ppappa1intintfMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppappa1intintfMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppappa1intintfMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppappa1intintfMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpappa1intintfMetricsIterator returns an iterator
func NewPpappa1intintfMetricsIterator() (*Ppappa1intintfMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppappa1intintfMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppappa1intintfMetricsIterator{iter: iter}, nil
}

type Sgite0interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgite0interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgite0interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

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
func (mtr *Sgite0interrMetrics) Unmarshal() error {
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
func (mtr *Sgite0interrMetrics) getOffset(fldName string) int {
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
func (mtr *Sgite0interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Sgite0interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Sgite0interrMetricsIterator is the iterator object
type Sgite0interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgite0interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgite0interrMetricsIterator) Next() *Sgite0interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgite0interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgite0interrMetricsIterator) Find(key uint64) (*Sgite0interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgite0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgite0interrMetricsIterator) Create(key uint64) (*Sgite0interrMetrics, error) {
	tmtr := &Sgite0interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgite0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgite0interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgite0interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgite0interrMetricsIterator returns an iterator
func NewSgite0interrMetricsIterator() (*Sgite0interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgite0interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgite0interrMetricsIterator{iter: iter}, nil
}

type Sgite1interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgite1interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgite1interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

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
func (mtr *Sgite1interrMetrics) Unmarshal() error {
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
func (mtr *Sgite1interrMetrics) getOffset(fldName string) int {
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
func (mtr *Sgite1interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Sgite1interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Sgite1interrMetricsIterator is the iterator object
type Sgite1interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgite1interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgite1interrMetricsIterator) Next() *Sgite1interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgite1interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgite1interrMetricsIterator) Find(key uint64) (*Sgite1interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgite1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgite1interrMetricsIterator) Create(key uint64) (*Sgite1interrMetrics, error) {
	tmtr := &Sgite1interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgite1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgite1interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgite1interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgite1interrMetricsIterator returns an iterator
func NewSgite1interrMetricsIterator() (*Sgite1interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgite1interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgite1interrMetricsIterator{iter: iter}, nil
}

type Sgite2interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgite2interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgite2interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

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
func (mtr *Sgite2interrMetrics) Unmarshal() error {
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
func (mtr *Sgite2interrMetrics) getOffset(fldName string) int {
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
func (mtr *Sgite2interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Sgite2interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Sgite2interrMetricsIterator is the iterator object
type Sgite2interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgite2interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgite2interrMetricsIterator) Next() *Sgite2interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgite2interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgite2interrMetricsIterator) Find(key uint64) (*Sgite2interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgite2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgite2interrMetricsIterator) Create(key uint64) (*Sgite2interrMetrics, error) {
	tmtr := &Sgite2interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgite2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgite2interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgite2interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgite2interrMetricsIterator returns an iterator
func NewSgite2interrMetricsIterator() (*Sgite2interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgite2interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgite2interrMetricsIterator{iter: iter}, nil
}

type Sgite3interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgite3interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgite3interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

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
func (mtr *Sgite3interrMetrics) Unmarshal() error {
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
func (mtr *Sgite3interrMetrics) getOffset(fldName string) int {
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
func (mtr *Sgite3interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Sgite3interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Sgite3interrMetricsIterator is the iterator object
type Sgite3interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgite3interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgite3interrMetricsIterator) Next() *Sgite3interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgite3interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgite3interrMetricsIterator) Find(key uint64) (*Sgite3interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgite3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgite3interrMetricsIterator) Create(key uint64) (*Sgite3interrMetrics, error) {
	tmtr := &Sgite3interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgite3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgite3interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgite3interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgite3interrMetricsIterator returns an iterator
func NewSgite3interrMetricsIterator() (*Sgite3interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgite3interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgite3interrMetricsIterator{iter: iter}, nil
}

type Sgite4interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgite4interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgite4interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

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
func (mtr *Sgite4interrMetrics) Unmarshal() error {
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
func (mtr *Sgite4interrMetrics) getOffset(fldName string) int {
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
func (mtr *Sgite4interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Sgite4interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Sgite4interrMetricsIterator is the iterator object
type Sgite4interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgite4interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgite4interrMetricsIterator) Next() *Sgite4interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgite4interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgite4interrMetricsIterator) Find(key uint64) (*Sgite4interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgite4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgite4interrMetricsIterator) Create(key uint64) (*Sgite4interrMetrics, error) {
	tmtr := &Sgite4interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgite4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgite4interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgite4interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgite4interrMetricsIterator returns an iterator
func NewSgite4interrMetricsIterator() (*Sgite4interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgite4interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgite4interrMetricsIterator{iter: iter}, nil
}

type Sgite5interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgite5interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgite5interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

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
func (mtr *Sgite5interrMetrics) Unmarshal() error {
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
func (mtr *Sgite5interrMetrics) getOffset(fldName string) int {
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
func (mtr *Sgite5interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Sgite5interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Sgite5interrMetricsIterator is the iterator object
type Sgite5interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgite5interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgite5interrMetricsIterator) Next() *Sgite5interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgite5interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgite5interrMetricsIterator) Find(key uint64) (*Sgite5interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgite5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgite5interrMetricsIterator) Create(key uint64) (*Sgite5interrMetrics, error) {
	tmtr := &Sgite5interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgite5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgite5interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgite5interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgite5interrMetricsIterator returns an iterator
func NewSgite5interrMetricsIterator() (*Sgite5interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgite5interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgite5interrMetricsIterator{iter: iter}, nil
}

type Sgimpu0interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgimpu0interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgimpu0interrMetrics) Size() int {
	sz := 0

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
func (mtr *Sgimpu0interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgimpu0interrMetrics) getOffset(fldName string) int {
	var offset int

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

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Sgimpu0interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Sgimpu0interrMetricsIterator is the iterator object
type Sgimpu0interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgimpu0interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgimpu0interrMetricsIterator) Next() *Sgimpu0interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgimpu0interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgimpu0interrMetricsIterator) Find(key uint64) (*Sgimpu0interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgimpu0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgimpu0interrMetricsIterator) Create(key uint64) (*Sgimpu0interrMetrics, error) {
	tmtr := &Sgimpu0interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgimpu0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgimpu0interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgimpu0interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgimpu0interrMetricsIterator returns an iterator
func NewSgimpu0interrMetricsIterator() (*Sgimpu0interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgimpu0interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgimpu0interrMetricsIterator{iter: iter}, nil
}

type Sgimpu0intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgimpu0intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgimpu0intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Sgimpu0intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgimpu0intinfoMetrics) getOffset(fldName string) int {
	var offset int

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

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Sgimpu0intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Sgimpu0intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Sgimpu0intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Sgimpu0intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Sgimpu0intinfoMetricsIterator is the iterator object
type Sgimpu0intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgimpu0intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgimpu0intinfoMetricsIterator) Next() *Sgimpu0intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgimpu0intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgimpu0intinfoMetricsIterator) Find(key uint64) (*Sgimpu0intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgimpu0intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgimpu0intinfoMetricsIterator) Create(key uint64) (*Sgimpu0intinfoMetrics, error) {
	tmtr := &Sgimpu0intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgimpu0intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgimpu0intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgimpu0intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgimpu0intinfoMetricsIterator returns an iterator
func NewSgimpu0intinfoMetricsIterator() (*Sgimpu0intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgimpu0intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgimpu0intinfoMetricsIterator{iter: iter}, nil
}

type Sgimpu1interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgimpu1interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgimpu1interrMetrics) Size() int {
	sz := 0

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
func (mtr *Sgimpu1interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgimpu1interrMetrics) getOffset(fldName string) int {
	var offset int

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

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Sgimpu1interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Sgimpu1interrMetricsIterator is the iterator object
type Sgimpu1interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgimpu1interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgimpu1interrMetricsIterator) Next() *Sgimpu1interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgimpu1interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgimpu1interrMetricsIterator) Find(key uint64) (*Sgimpu1interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgimpu1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgimpu1interrMetricsIterator) Create(key uint64) (*Sgimpu1interrMetrics, error) {
	tmtr := &Sgimpu1interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgimpu1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgimpu1interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgimpu1interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgimpu1interrMetricsIterator returns an iterator
func NewSgimpu1interrMetricsIterator() (*Sgimpu1interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgimpu1interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgimpu1interrMetricsIterator{iter: iter}, nil
}

type Sgimpu1intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgimpu1intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgimpu1intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Sgimpu1intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgimpu1intinfoMetrics) getOffset(fldName string) int {
	var offset int

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

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Sgimpu1intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Sgimpu1intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Sgimpu1intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Sgimpu1intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Sgimpu1intinfoMetricsIterator is the iterator object
type Sgimpu1intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgimpu1intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgimpu1intinfoMetricsIterator) Next() *Sgimpu1intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgimpu1intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgimpu1intinfoMetricsIterator) Find(key uint64) (*Sgimpu1intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgimpu1intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgimpu1intinfoMetricsIterator) Create(key uint64) (*Sgimpu1intinfoMetrics, error) {
	tmtr := &Sgimpu1intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgimpu1intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgimpu1intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgimpu1intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgimpu1intinfoMetricsIterator returns an iterator
func NewSgimpu1intinfoMetricsIterator() (*Sgimpu1intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgimpu1intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgimpu1intinfoMetricsIterator{iter: iter}, nil
}

type Sgimpu2interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgimpu2interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgimpu2interrMetrics) Size() int {
	sz := 0

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
func (mtr *Sgimpu2interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgimpu2interrMetrics) getOffset(fldName string) int {
	var offset int

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

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Sgimpu2interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Sgimpu2interrMetricsIterator is the iterator object
type Sgimpu2interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgimpu2interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgimpu2interrMetricsIterator) Next() *Sgimpu2interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgimpu2interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgimpu2interrMetricsIterator) Find(key uint64) (*Sgimpu2interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgimpu2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgimpu2interrMetricsIterator) Create(key uint64) (*Sgimpu2interrMetrics, error) {
	tmtr := &Sgimpu2interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgimpu2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgimpu2interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgimpu2interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgimpu2interrMetricsIterator returns an iterator
func NewSgimpu2interrMetricsIterator() (*Sgimpu2interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgimpu2interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgimpu2interrMetricsIterator{iter: iter}, nil
}

type Sgimpu2intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgimpu2intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgimpu2intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Sgimpu2intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgimpu2intinfoMetrics) getOffset(fldName string) int {
	var offset int

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

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Sgimpu2intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Sgimpu2intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Sgimpu2intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Sgimpu2intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Sgimpu2intinfoMetricsIterator is the iterator object
type Sgimpu2intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgimpu2intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgimpu2intinfoMetricsIterator) Next() *Sgimpu2intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgimpu2intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgimpu2intinfoMetricsIterator) Find(key uint64) (*Sgimpu2intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgimpu2intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgimpu2intinfoMetricsIterator) Create(key uint64) (*Sgimpu2intinfoMetrics, error) {
	tmtr := &Sgimpu2intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgimpu2intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgimpu2intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgimpu2intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgimpu2intinfoMetricsIterator returns an iterator
func NewSgimpu2intinfoMetricsIterator() (*Sgimpu2intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgimpu2intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgimpu2intinfoMetricsIterator{iter: iter}, nil
}

type Sgimpu3interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgimpu3interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgimpu3interrMetrics) Size() int {
	sz := 0

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
func (mtr *Sgimpu3interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgimpu3interrMetrics) getOffset(fldName string) int {
	var offset int

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

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Sgimpu3interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Sgimpu3interrMetricsIterator is the iterator object
type Sgimpu3interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgimpu3interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgimpu3interrMetricsIterator) Next() *Sgimpu3interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgimpu3interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgimpu3interrMetricsIterator) Find(key uint64) (*Sgimpu3interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgimpu3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgimpu3interrMetricsIterator) Create(key uint64) (*Sgimpu3interrMetrics, error) {
	tmtr := &Sgimpu3interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgimpu3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgimpu3interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgimpu3interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgimpu3interrMetricsIterator returns an iterator
func NewSgimpu3interrMetricsIterator() (*Sgimpu3interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgimpu3interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgimpu3interrMetricsIterator{iter: iter}, nil
}

type Sgimpu3intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgimpu3intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgimpu3intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Sgimpu3intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgimpu3intinfoMetrics) getOffset(fldName string) int {
	var offset int

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

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Sgimpu3intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Sgimpu3intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Sgimpu3intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Sgimpu3intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Sgimpu3intinfoMetricsIterator is the iterator object
type Sgimpu3intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgimpu3intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgimpu3intinfoMetricsIterator) Next() *Sgimpu3intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgimpu3intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgimpu3intinfoMetricsIterator) Find(key uint64) (*Sgimpu3intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgimpu3intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgimpu3intinfoMetricsIterator) Create(key uint64) (*Sgimpu3intinfoMetrics, error) {
	tmtr := &Sgimpu3intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgimpu3intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgimpu3intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgimpu3intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgimpu3intinfoMetricsIterator returns an iterator
func NewSgimpu3intinfoMetricsIterator() (*Sgimpu3intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgimpu3intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgimpu3intinfoMetricsIterator{iter: iter}, nil
}

type Sgimpu4interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgimpu4interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgimpu4interrMetrics) Size() int {
	sz := 0

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
func (mtr *Sgimpu4interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgimpu4interrMetrics) getOffset(fldName string) int {
	var offset int

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

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Sgimpu4interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Sgimpu4interrMetricsIterator is the iterator object
type Sgimpu4interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgimpu4interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgimpu4interrMetricsIterator) Next() *Sgimpu4interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgimpu4interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgimpu4interrMetricsIterator) Find(key uint64) (*Sgimpu4interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgimpu4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgimpu4interrMetricsIterator) Create(key uint64) (*Sgimpu4interrMetrics, error) {
	tmtr := &Sgimpu4interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgimpu4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgimpu4interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgimpu4interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgimpu4interrMetricsIterator returns an iterator
func NewSgimpu4interrMetricsIterator() (*Sgimpu4interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgimpu4interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgimpu4interrMetricsIterator{iter: iter}, nil
}

type Sgimpu4intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgimpu4intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgimpu4intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Sgimpu4intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgimpu4intinfoMetrics) getOffset(fldName string) int {
	var offset int

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

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Sgimpu4intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Sgimpu4intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Sgimpu4intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Sgimpu4intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Sgimpu4intinfoMetricsIterator is the iterator object
type Sgimpu4intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgimpu4intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgimpu4intinfoMetricsIterator) Next() *Sgimpu4intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgimpu4intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgimpu4intinfoMetricsIterator) Find(key uint64) (*Sgimpu4intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgimpu4intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgimpu4intinfoMetricsIterator) Create(key uint64) (*Sgimpu4intinfoMetrics, error) {
	tmtr := &Sgimpu4intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgimpu4intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgimpu4intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgimpu4intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgimpu4intinfoMetricsIterator returns an iterator
func NewSgimpu4intinfoMetricsIterator() (*Sgimpu4intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgimpu4intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgimpu4intinfoMetricsIterator{iter: iter}, nil
}

type Sgimpu5interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgimpu5interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgimpu5interrMetrics) Size() int {
	sz := 0

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
func (mtr *Sgimpu5interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgimpu5interrMetrics) getOffset(fldName string) int {
	var offset int

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

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Sgimpu5interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Sgimpu5interrMetricsIterator is the iterator object
type Sgimpu5interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgimpu5interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgimpu5interrMetricsIterator) Next() *Sgimpu5interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgimpu5interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgimpu5interrMetricsIterator) Find(key uint64) (*Sgimpu5interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgimpu5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgimpu5interrMetricsIterator) Create(key uint64) (*Sgimpu5interrMetrics, error) {
	tmtr := &Sgimpu5interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgimpu5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgimpu5interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgimpu5interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgimpu5interrMetricsIterator returns an iterator
func NewSgimpu5interrMetricsIterator() (*Sgimpu5interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgimpu5interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgimpu5interrMetricsIterator{iter: iter}, nil
}

type Sgimpu5intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgimpu5intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgimpu5intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Sgimpu5intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgimpu5intinfoMetrics) getOffset(fldName string) int {
	var offset int

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

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Sgimpu5intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Sgimpu5intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Sgimpu5intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Sgimpu5intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Sgimpu5intinfoMetricsIterator is the iterator object
type Sgimpu5intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgimpu5intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgimpu5intinfoMetricsIterator) Next() *Sgimpu5intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgimpu5intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgimpu5intinfoMetricsIterator) Find(key uint64) (*Sgimpu5intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgimpu5intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgimpu5intinfoMetricsIterator) Create(key uint64) (*Sgimpu5intinfoMetrics, error) {
	tmtr := &Sgimpu5intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgimpu5intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgimpu5intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgimpu5intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgimpu5intinfoMetricsIterator returns an iterator
func NewSgimpu5intinfoMetricsIterator() (*Sgimpu5intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgimpu5intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgimpu5intinfoMetricsIterator{iter: iter}, nil
}

type RpcpicsintpicsMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	UncorrectableEcc metrics.Counter

	CorrectableEcc metrics.Counter

	TooManyRlSchError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *RpcpicsintpicsMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *RpcpicsintpicsMetrics) Size() int {
	sz := 0

	sz += mtr.UncorrectableEcc.Size()

	sz += mtr.CorrectableEcc.Size()

	sz += mtr.TooManyRlSchError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *RpcpicsintpicsMetrics) Unmarshal() error {
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
func (mtr *RpcpicsintpicsMetrics) getOffset(fldName string) int {
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
func (mtr *RpcpicsintpicsMetrics) SetUncorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UncorrectableEcc"))
	return nil
}

// SetCorrectableEcc sets cunter in shared memory
func (mtr *RpcpicsintpicsMetrics) SetCorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CorrectableEcc"))
	return nil
}

// SetTooManyRlSchError sets cunter in shared memory
func (mtr *RpcpicsintpicsMetrics) SetTooManyRlSchError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TooManyRlSchError"))
	return nil
}

// RpcpicsintpicsMetricsIterator is the iterator object
type RpcpicsintpicsMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *RpcpicsintpicsMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *RpcpicsintpicsMetricsIterator) Next() *RpcpicsintpicsMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &RpcpicsintpicsMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *RpcpicsintpicsMetricsIterator) Find(key uint64) (*RpcpicsintpicsMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &RpcpicsintpicsMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *RpcpicsintpicsMetricsIterator) Create(key uint64) (*RpcpicsintpicsMetrics, error) {
	tmtr := &RpcpicsintpicsMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &RpcpicsintpicsMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *RpcpicsintpicsMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *RpcpicsintpicsMetricsIterator) Free() {
	it.iter.Free()
}

// NewRpcpicsintpicsMetricsIterator returns an iterator
func NewRpcpicsintpicsMetricsIterator() (*RpcpicsintpicsMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("RpcpicsintpicsMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &RpcpicsintpicsMetricsIterator{iter: iter}, nil
}

type RpcpicsintbadaddrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Rdreq0BadAddr metrics.Counter

	Wrreq0BadAddr metrics.Counter

	Rdreq1BadAddr metrics.Counter

	Wrreq1BadAddr metrics.Counter

	Rdreq2BadAddr metrics.Counter

	Wrreq2BadAddr metrics.Counter

	Rdreq3BadAddr metrics.Counter

	Wrreq3BadAddr metrics.Counter

	Rdreq4BadAddr metrics.Counter

	Wrreq4BadAddr metrics.Counter

	Rdreq5BadAddr metrics.Counter

	Wrreq5BadAddr metrics.Counter

	Rdreq6BadAddr metrics.Counter

	Wrreq6BadAddr metrics.Counter

	Rdreq7BadAddr metrics.Counter

	Wrreq7BadAddr metrics.Counter

	CpuBadAddr metrics.Counter

	BgBadAddr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *RpcpicsintbadaddrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *RpcpicsintbadaddrMetrics) Size() int {
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
func (mtr *RpcpicsintbadaddrMetrics) Unmarshal() error {
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
func (mtr *RpcpicsintbadaddrMetrics) getOffset(fldName string) int {
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
func (mtr *RpcpicsintbadaddrMetrics) SetRdreq0BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq0BadAddr"))
	return nil
}

// SetWrreq0BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetWrreq0BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq0BadAddr"))
	return nil
}

// SetRdreq1BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetRdreq1BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq1BadAddr"))
	return nil
}

// SetWrreq1BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetWrreq1BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq1BadAddr"))
	return nil
}

// SetRdreq2BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetRdreq2BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq2BadAddr"))
	return nil
}

// SetWrreq2BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetWrreq2BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq2BadAddr"))
	return nil
}

// SetRdreq3BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetRdreq3BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq3BadAddr"))
	return nil
}

// SetWrreq3BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetWrreq3BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq3BadAddr"))
	return nil
}

// SetRdreq4BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetRdreq4BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq4BadAddr"))
	return nil
}

// SetWrreq4BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetWrreq4BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq4BadAddr"))
	return nil
}

// SetRdreq5BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetRdreq5BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq5BadAddr"))
	return nil
}

// SetWrreq5BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetWrreq5BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq5BadAddr"))
	return nil
}

// SetRdreq6BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetRdreq6BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq6BadAddr"))
	return nil
}

// SetWrreq6BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetWrreq6BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq6BadAddr"))
	return nil
}

// SetRdreq7BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetRdreq7BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq7BadAddr"))
	return nil
}

// SetWrreq7BadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetWrreq7BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq7BadAddr"))
	return nil
}

// SetCpuBadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetCpuBadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CpuBadAddr"))
	return nil
}

// SetBgBadAddr sets cunter in shared memory
func (mtr *RpcpicsintbadaddrMetrics) SetBgBadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BgBadAddr"))
	return nil
}

// RpcpicsintbadaddrMetricsIterator is the iterator object
type RpcpicsintbadaddrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *RpcpicsintbadaddrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *RpcpicsintbadaddrMetricsIterator) Next() *RpcpicsintbadaddrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &RpcpicsintbadaddrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *RpcpicsintbadaddrMetricsIterator) Find(key uint64) (*RpcpicsintbadaddrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &RpcpicsintbadaddrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *RpcpicsintbadaddrMetricsIterator) Create(key uint64) (*RpcpicsintbadaddrMetrics, error) {
	tmtr := &RpcpicsintbadaddrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &RpcpicsintbadaddrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *RpcpicsintbadaddrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *RpcpicsintbadaddrMetricsIterator) Free() {
	it.iter.Free()
}

// NewRpcpicsintbadaddrMetricsIterator returns an iterator
func NewRpcpicsintbadaddrMetricsIterator() (*RpcpicsintbadaddrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("RpcpicsintbadaddrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &RpcpicsintbadaddrMetricsIterator{iter: iter}, nil
}

type RpcpicsintbgMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	UnfinishedBg0 metrics.Counter

	UnfinishedBg1 metrics.Counter

	UnfinishedBg2 metrics.Counter

	UnfinishedBg3 metrics.Counter

	UnfinishedBg4 metrics.Counter

	UnfinishedBg5 metrics.Counter

	UnfinishedBg6 metrics.Counter

	UnfinishedBg7 metrics.Counter

	UnfinishedBg8 metrics.Counter

	UnfinishedBg9 metrics.Counter

	UnfinishedBg10 metrics.Counter

	UnfinishedBg11 metrics.Counter

	UnfinishedBg12 metrics.Counter

	UnfinishedBg13 metrics.Counter

	UnfinishedBg14 metrics.Counter

	UnfinishedBg15 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *RpcpicsintbgMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *RpcpicsintbgMetrics) Size() int {
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
func (mtr *RpcpicsintbgMetrics) Unmarshal() error {
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
func (mtr *RpcpicsintbgMetrics) getOffset(fldName string) int {
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
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg0"))
	return nil
}

// SetUnfinishedBg1 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg1"))
	return nil
}

// SetUnfinishedBg2 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg2"))
	return nil
}

// SetUnfinishedBg3 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg3"))
	return nil
}

// SetUnfinishedBg4 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg4"))
	return nil
}

// SetUnfinishedBg5 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg5"))
	return nil
}

// SetUnfinishedBg6 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg6"))
	return nil
}

// SetUnfinishedBg7 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg7"))
	return nil
}

// SetUnfinishedBg8 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg8"))
	return nil
}

// SetUnfinishedBg9 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg9"))
	return nil
}

// SetUnfinishedBg10 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg10"))
	return nil
}

// SetUnfinishedBg11 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg11"))
	return nil
}

// SetUnfinishedBg12 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg12(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg12"))
	return nil
}

// SetUnfinishedBg13 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg13(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg13"))
	return nil
}

// SetUnfinishedBg14 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg14(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg14"))
	return nil
}

// SetUnfinishedBg15 sets cunter in shared memory
func (mtr *RpcpicsintbgMetrics) SetUnfinishedBg15(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg15"))
	return nil
}

// RpcpicsintbgMetricsIterator is the iterator object
type RpcpicsintbgMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *RpcpicsintbgMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *RpcpicsintbgMetricsIterator) Next() *RpcpicsintbgMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &RpcpicsintbgMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *RpcpicsintbgMetricsIterator) Find(key uint64) (*RpcpicsintbgMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &RpcpicsintbgMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *RpcpicsintbgMetricsIterator) Create(key uint64) (*RpcpicsintbgMetrics, error) {
	tmtr := &RpcpicsintbgMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &RpcpicsintbgMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *RpcpicsintbgMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *RpcpicsintbgMetricsIterator) Free() {
	it.iter.Free()
}

// NewRpcpicsintbgMetricsIterator returns an iterator
func NewRpcpicsintbgMetricsIterator() (*RpcpicsintbgMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("RpcpicsintbgMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &RpcpicsintbgMetricsIterator{iter: iter}, nil
}

type RpcpicspiccintpiccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	UncorrectableEcc metrics.Counter

	CorrectableEcc metrics.Counter

	CacheStgAwlenErr metrics.Counter

	CacheHbmRrespSlverr metrics.Counter

	CacheHbmRrespDecerr metrics.Counter

	CacheHbmBrespSlverr metrics.Counter

	CacheHbmBrespDecerr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *RpcpicspiccintpiccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *RpcpicspiccintpiccMetrics) Size() int {
	sz := 0

	sz += mtr.UncorrectableEcc.Size()

	sz += mtr.CorrectableEcc.Size()

	sz += mtr.CacheStgAwlenErr.Size()

	sz += mtr.CacheHbmRrespSlverr.Size()

	sz += mtr.CacheHbmRrespDecerr.Size()

	sz += mtr.CacheHbmBrespSlverr.Size()

	sz += mtr.CacheHbmBrespDecerr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *RpcpicspiccintpiccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.UncorrectableEcc = mtr.metrics.GetCounter(offset)
	offset += mtr.UncorrectableEcc.Size()

	mtr.CorrectableEcc = mtr.metrics.GetCounter(offset)
	offset += mtr.CorrectableEcc.Size()

	mtr.CacheStgAwlenErr = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheStgAwlenErr.Size()

	mtr.CacheHbmRrespSlverr = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheHbmRrespSlverr.Size()

	mtr.CacheHbmRrespDecerr = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheHbmRrespDecerr.Size()

	mtr.CacheHbmBrespSlverr = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheHbmBrespSlverr.Size()

	mtr.CacheHbmBrespDecerr = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheHbmBrespDecerr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *RpcpicspiccintpiccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "UncorrectableEcc" {
		return offset
	}
	offset += mtr.UncorrectableEcc.Size()

	if fldName == "CorrectableEcc" {
		return offset
	}
	offset += mtr.CorrectableEcc.Size()

	if fldName == "CacheStgAwlenErr" {
		return offset
	}
	offset += mtr.CacheStgAwlenErr.Size()

	if fldName == "CacheHbmRrespSlverr" {
		return offset
	}
	offset += mtr.CacheHbmRrespSlverr.Size()

	if fldName == "CacheHbmRrespDecerr" {
		return offset
	}
	offset += mtr.CacheHbmRrespDecerr.Size()

	if fldName == "CacheHbmBrespSlverr" {
		return offset
	}
	offset += mtr.CacheHbmBrespSlverr.Size()

	if fldName == "CacheHbmBrespDecerr" {
		return offset
	}
	offset += mtr.CacheHbmBrespDecerr.Size()

	return offset
}

// SetUncorrectableEcc sets cunter in shared memory
func (mtr *RpcpicspiccintpiccMetrics) SetUncorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UncorrectableEcc"))
	return nil
}

// SetCorrectableEcc sets cunter in shared memory
func (mtr *RpcpicspiccintpiccMetrics) SetCorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CorrectableEcc"))
	return nil
}

// SetCacheStgAwlenErr sets cunter in shared memory
func (mtr *RpcpicspiccintpiccMetrics) SetCacheStgAwlenErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheStgAwlenErr"))
	return nil
}

// SetCacheHbmRrespSlverr sets cunter in shared memory
func (mtr *RpcpicspiccintpiccMetrics) SetCacheHbmRrespSlverr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmRrespSlverr"))
	return nil
}

// SetCacheHbmRrespDecerr sets cunter in shared memory
func (mtr *RpcpicspiccintpiccMetrics) SetCacheHbmRrespDecerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmRrespDecerr"))
	return nil
}

// SetCacheHbmBrespSlverr sets cunter in shared memory
func (mtr *RpcpicspiccintpiccMetrics) SetCacheHbmBrespSlverr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmBrespSlverr"))
	return nil
}

// SetCacheHbmBrespDecerr sets cunter in shared memory
func (mtr *RpcpicspiccintpiccMetrics) SetCacheHbmBrespDecerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmBrespDecerr"))
	return nil
}

// RpcpicspiccintpiccMetricsIterator is the iterator object
type RpcpicspiccintpiccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *RpcpicspiccintpiccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *RpcpicspiccintpiccMetricsIterator) Next() *RpcpicspiccintpiccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &RpcpicspiccintpiccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *RpcpicspiccintpiccMetricsIterator) Find(key uint64) (*RpcpicspiccintpiccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &RpcpicspiccintpiccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *RpcpicspiccintpiccMetricsIterator) Create(key uint64) (*RpcpicspiccintpiccMetrics, error) {
	tmtr := &RpcpicspiccintpiccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &RpcpicspiccintpiccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *RpcpicspiccintpiccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *RpcpicspiccintpiccMetricsIterator) Free() {
	it.iter.Free()
}

// NewRpcpicspiccintpiccMetricsIterator returns an iterator
func NewRpcpicspiccintpiccMetricsIterator() (*RpcpicspiccintpiccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("RpcpicspiccintpiccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &RpcpicspiccintpiccMetricsIterator{iter: iter}, nil
}

type IntrintrintintreccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	IntrStateUncorrectable metrics.Counter

	IntrStateCorrectable metrics.Counter

	IntrPbaCfgUncorrectable metrics.Counter

	IntrPbaCfgCorrectable metrics.Counter

	IntrCoalesceUncorrectable metrics.Counter

	IntrCoalesceCorrectable metrics.Counter

	IntrLegacyFifoOverrun metrics.Counter

	IntrBrespErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *IntrintrintintreccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *IntrintrintintreccMetrics) Size() int {
	sz := 0

	sz += mtr.IntrStateUncorrectable.Size()

	sz += mtr.IntrStateCorrectable.Size()

	sz += mtr.IntrPbaCfgUncorrectable.Size()

	sz += mtr.IntrPbaCfgCorrectable.Size()

	sz += mtr.IntrCoalesceUncorrectable.Size()

	sz += mtr.IntrCoalesceCorrectable.Size()

	sz += mtr.IntrLegacyFifoOverrun.Size()

	sz += mtr.IntrBrespErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *IntrintrintintreccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.IntrStateUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrStateUncorrectable.Size()

	mtr.IntrStateCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrStateCorrectable.Size()

	mtr.IntrPbaCfgUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrPbaCfgUncorrectable.Size()

	mtr.IntrPbaCfgCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrPbaCfgCorrectable.Size()

	mtr.IntrCoalesceUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrCoalesceUncorrectable.Size()

	mtr.IntrCoalesceCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrCoalesceCorrectable.Size()

	mtr.IntrLegacyFifoOverrun = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrLegacyFifoOverrun.Size()

	mtr.IntrBrespErr = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrBrespErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *IntrintrintintreccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "IntrStateUncorrectable" {
		return offset
	}
	offset += mtr.IntrStateUncorrectable.Size()

	if fldName == "IntrStateCorrectable" {
		return offset
	}
	offset += mtr.IntrStateCorrectable.Size()

	if fldName == "IntrPbaCfgUncorrectable" {
		return offset
	}
	offset += mtr.IntrPbaCfgUncorrectable.Size()

	if fldName == "IntrPbaCfgCorrectable" {
		return offset
	}
	offset += mtr.IntrPbaCfgCorrectable.Size()

	if fldName == "IntrCoalesceUncorrectable" {
		return offset
	}
	offset += mtr.IntrCoalesceUncorrectable.Size()

	if fldName == "IntrCoalesceCorrectable" {
		return offset
	}
	offset += mtr.IntrCoalesceCorrectable.Size()

	if fldName == "IntrLegacyFifoOverrun" {
		return offset
	}
	offset += mtr.IntrLegacyFifoOverrun.Size()

	if fldName == "IntrBrespErr" {
		return offset
	}
	offset += mtr.IntrBrespErr.Size()

	return offset
}

// SetIntrStateUncorrectable sets cunter in shared memory
func (mtr *IntrintrintintreccMetrics) SetIntrStateUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrStateUncorrectable"))
	return nil
}

// SetIntrStateCorrectable sets cunter in shared memory
func (mtr *IntrintrintintreccMetrics) SetIntrStateCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrStateCorrectable"))
	return nil
}

// SetIntrPbaCfgUncorrectable sets cunter in shared memory
func (mtr *IntrintrintintreccMetrics) SetIntrPbaCfgUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrPbaCfgUncorrectable"))
	return nil
}

// SetIntrPbaCfgCorrectable sets cunter in shared memory
func (mtr *IntrintrintintreccMetrics) SetIntrPbaCfgCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrPbaCfgCorrectable"))
	return nil
}

// SetIntrCoalesceUncorrectable sets cunter in shared memory
func (mtr *IntrintrintintreccMetrics) SetIntrCoalesceUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrCoalesceUncorrectable"))
	return nil
}

// SetIntrCoalesceCorrectable sets cunter in shared memory
func (mtr *IntrintrintintreccMetrics) SetIntrCoalesceCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrCoalesceCorrectable"))
	return nil
}

// SetIntrLegacyFifoOverrun sets cunter in shared memory
func (mtr *IntrintrintintreccMetrics) SetIntrLegacyFifoOverrun(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrLegacyFifoOverrun"))
	return nil
}

// SetIntrBrespErr sets cunter in shared memory
func (mtr *IntrintrintintreccMetrics) SetIntrBrespErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrBrespErr"))
	return nil
}

// IntrintrintintreccMetricsIterator is the iterator object
type IntrintrintintreccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *IntrintrintintreccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *IntrintrintintreccMetricsIterator) Next() *IntrintrintintreccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &IntrintrintintreccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *IntrintrintintreccMetricsIterator) Find(key uint64) (*IntrintrintintreccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &IntrintrintintreccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *IntrintrintintreccMetricsIterator) Create(key uint64) (*IntrintrintintreccMetrics, error) {
	tmtr := &IntrintrintintreccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &IntrintrintintreccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *IntrintrintintreccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *IntrintrintintreccMetricsIterator) Free() {
	it.iter.Free()
}

// NewIntrintrintintreccMetricsIterator returns an iterator
func NewIntrintrintintreccMetricsIterator() (*IntrintrintintreccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("IntrintrintintreccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &IntrintrintintreccMetricsIterator{iter: iter}, nil
}

type PxbpxbinttgteccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Aximst_0Uncorrectable metrics.Counter

	Aximst_0Correctable metrics.Counter

	Aximst_1Uncorrectable metrics.Counter

	Aximst_1Correctable metrics.Counter

	Aximst_2Uncorrectable metrics.Counter

	Aximst_2Correctable metrics.Counter

	Aximst_3Uncorrectable metrics.Counter

	Aximst_3Correctable metrics.Counter

	Aximst_4Uncorrectable metrics.Counter

	Aximst_4Correctable metrics.Counter

	Rxcrbfr_0Uncorrectable metrics.Counter

	Rxcrbfr_0Correctable metrics.Counter

	Rxcrbfr_1Uncorrectable metrics.Counter

	Rxcrbfr_1Correctable metrics.Counter

	Rxcrbfr_2Uncorrectable metrics.Counter

	Rxcrbfr_2Correctable metrics.Counter

	Rxcrbfr_3Uncorrectable metrics.Counter

	Rxcrbfr_3Correctable metrics.Counter

	RxinfoUncorrectable metrics.Counter

	RxinfoCorrectable metrics.Counter

	CplstUncorrectable metrics.Counter

	CplstCorrectable metrics.Counter

	RomaskUncorrectable metrics.Counter

	RomaskCorrectable metrics.Counter

	PmrUncorrectable metrics.Counter

	PmrCorrectable metrics.Counter

	PrtUncorrectable metrics.Counter

	PrtCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PxbpxbinttgteccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PxbpxbinttgteccMetrics) Size() int {
	sz := 0

	sz += mtr.Aximst_0Uncorrectable.Size()

	sz += mtr.Aximst_0Correctable.Size()

	sz += mtr.Aximst_1Uncorrectable.Size()

	sz += mtr.Aximst_1Correctable.Size()

	sz += mtr.Aximst_2Uncorrectable.Size()

	sz += mtr.Aximst_2Correctable.Size()

	sz += mtr.Aximst_3Uncorrectable.Size()

	sz += mtr.Aximst_3Correctable.Size()

	sz += mtr.Aximst_4Uncorrectable.Size()

	sz += mtr.Aximst_4Correctable.Size()

	sz += mtr.Rxcrbfr_0Uncorrectable.Size()

	sz += mtr.Rxcrbfr_0Correctable.Size()

	sz += mtr.Rxcrbfr_1Uncorrectable.Size()

	sz += mtr.Rxcrbfr_1Correctable.Size()

	sz += mtr.Rxcrbfr_2Uncorrectable.Size()

	sz += mtr.Rxcrbfr_2Correctable.Size()

	sz += mtr.Rxcrbfr_3Uncorrectable.Size()

	sz += mtr.Rxcrbfr_3Correctable.Size()

	sz += mtr.RxinfoUncorrectable.Size()

	sz += mtr.RxinfoCorrectable.Size()

	sz += mtr.CplstUncorrectable.Size()

	sz += mtr.CplstCorrectable.Size()

	sz += mtr.RomaskUncorrectable.Size()

	sz += mtr.RomaskCorrectable.Size()

	sz += mtr.PmrUncorrectable.Size()

	sz += mtr.PmrCorrectable.Size()

	sz += mtr.PrtUncorrectable.Size()

	sz += mtr.PrtCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PxbpxbinttgteccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Aximst_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Aximst_0Uncorrectable.Size()

	mtr.Aximst_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Aximst_0Correctable.Size()

	mtr.Aximst_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Aximst_1Uncorrectable.Size()

	mtr.Aximst_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Aximst_1Correctable.Size()

	mtr.Aximst_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Aximst_2Uncorrectable.Size()

	mtr.Aximst_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Aximst_2Correctable.Size()

	mtr.Aximst_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Aximst_3Uncorrectable.Size()

	mtr.Aximst_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Aximst_3Correctable.Size()

	mtr.Aximst_4Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Aximst_4Uncorrectable.Size()

	mtr.Aximst_4Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Aximst_4Correctable.Size()

	mtr.Rxcrbfr_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxcrbfr_0Uncorrectable.Size()

	mtr.Rxcrbfr_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxcrbfr_0Correctable.Size()

	mtr.Rxcrbfr_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxcrbfr_1Uncorrectable.Size()

	mtr.Rxcrbfr_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxcrbfr_1Correctable.Size()

	mtr.Rxcrbfr_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxcrbfr_2Uncorrectable.Size()

	mtr.Rxcrbfr_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxcrbfr_2Correctable.Size()

	mtr.Rxcrbfr_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxcrbfr_3Uncorrectable.Size()

	mtr.Rxcrbfr_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxcrbfr_3Correctable.Size()

	mtr.RxinfoUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RxinfoUncorrectable.Size()

	mtr.RxinfoCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RxinfoCorrectable.Size()

	mtr.CplstUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.CplstUncorrectable.Size()

	mtr.CplstCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.CplstCorrectable.Size()

	mtr.RomaskUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RomaskUncorrectable.Size()

	mtr.RomaskCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RomaskCorrectable.Size()

	mtr.PmrUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PmrUncorrectable.Size()

	mtr.PmrCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PmrCorrectable.Size()

	mtr.PrtUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PrtUncorrectable.Size()

	mtr.PrtCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PrtCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PxbpxbinttgteccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Aximst_0Uncorrectable" {
		return offset
	}
	offset += mtr.Aximst_0Uncorrectable.Size()

	if fldName == "Aximst_0Correctable" {
		return offset
	}
	offset += mtr.Aximst_0Correctable.Size()

	if fldName == "Aximst_1Uncorrectable" {
		return offset
	}
	offset += mtr.Aximst_1Uncorrectable.Size()

	if fldName == "Aximst_1Correctable" {
		return offset
	}
	offset += mtr.Aximst_1Correctable.Size()

	if fldName == "Aximst_2Uncorrectable" {
		return offset
	}
	offset += mtr.Aximst_2Uncorrectable.Size()

	if fldName == "Aximst_2Correctable" {
		return offset
	}
	offset += mtr.Aximst_2Correctable.Size()

	if fldName == "Aximst_3Uncorrectable" {
		return offset
	}
	offset += mtr.Aximst_3Uncorrectable.Size()

	if fldName == "Aximst_3Correctable" {
		return offset
	}
	offset += mtr.Aximst_3Correctable.Size()

	if fldName == "Aximst_4Uncorrectable" {
		return offset
	}
	offset += mtr.Aximst_4Uncorrectable.Size()

	if fldName == "Aximst_4Correctable" {
		return offset
	}
	offset += mtr.Aximst_4Correctable.Size()

	if fldName == "Rxcrbfr_0Uncorrectable" {
		return offset
	}
	offset += mtr.Rxcrbfr_0Uncorrectable.Size()

	if fldName == "Rxcrbfr_0Correctable" {
		return offset
	}
	offset += mtr.Rxcrbfr_0Correctable.Size()

	if fldName == "Rxcrbfr_1Uncorrectable" {
		return offset
	}
	offset += mtr.Rxcrbfr_1Uncorrectable.Size()

	if fldName == "Rxcrbfr_1Correctable" {
		return offset
	}
	offset += mtr.Rxcrbfr_1Correctable.Size()

	if fldName == "Rxcrbfr_2Uncorrectable" {
		return offset
	}
	offset += mtr.Rxcrbfr_2Uncorrectable.Size()

	if fldName == "Rxcrbfr_2Correctable" {
		return offset
	}
	offset += mtr.Rxcrbfr_2Correctable.Size()

	if fldName == "Rxcrbfr_3Uncorrectable" {
		return offset
	}
	offset += mtr.Rxcrbfr_3Uncorrectable.Size()

	if fldName == "Rxcrbfr_3Correctable" {
		return offset
	}
	offset += mtr.Rxcrbfr_3Correctable.Size()

	if fldName == "RxinfoUncorrectable" {
		return offset
	}
	offset += mtr.RxinfoUncorrectable.Size()

	if fldName == "RxinfoCorrectable" {
		return offset
	}
	offset += mtr.RxinfoCorrectable.Size()

	if fldName == "CplstUncorrectable" {
		return offset
	}
	offset += mtr.CplstUncorrectable.Size()

	if fldName == "CplstCorrectable" {
		return offset
	}
	offset += mtr.CplstCorrectable.Size()

	if fldName == "RomaskUncorrectable" {
		return offset
	}
	offset += mtr.RomaskUncorrectable.Size()

	if fldName == "RomaskCorrectable" {
		return offset
	}
	offset += mtr.RomaskCorrectable.Size()

	if fldName == "PmrUncorrectable" {
		return offset
	}
	offset += mtr.PmrUncorrectable.Size()

	if fldName == "PmrCorrectable" {
		return offset
	}
	offset += mtr.PmrCorrectable.Size()

	if fldName == "PrtUncorrectable" {
		return offset
	}
	offset += mtr.PrtUncorrectable.Size()

	if fldName == "PrtCorrectable" {
		return offset
	}
	offset += mtr.PrtCorrectable.Size()

	return offset
}

// SetAximst_0Uncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetAximst_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Aximst_0Uncorrectable"))
	return nil
}

// SetAximst_0Correctable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetAximst_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Aximst_0Correctable"))
	return nil
}

// SetAximst_1Uncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetAximst_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Aximst_1Uncorrectable"))
	return nil
}

// SetAximst_1Correctable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetAximst_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Aximst_1Correctable"))
	return nil
}

// SetAximst_2Uncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetAximst_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Aximst_2Uncorrectable"))
	return nil
}

// SetAximst_2Correctable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetAximst_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Aximst_2Correctable"))
	return nil
}

// SetAximst_3Uncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetAximst_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Aximst_3Uncorrectable"))
	return nil
}

// SetAximst_3Correctable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetAximst_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Aximst_3Correctable"))
	return nil
}

// SetAximst_4Uncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetAximst_4Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Aximst_4Uncorrectable"))
	return nil
}

// SetAximst_4Correctable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetAximst_4Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Aximst_4Correctable"))
	return nil
}

// SetRxcrbfr_0Uncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetRxcrbfr_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxcrbfr_0Uncorrectable"))
	return nil
}

// SetRxcrbfr_0Correctable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetRxcrbfr_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxcrbfr_0Correctable"))
	return nil
}

// SetRxcrbfr_1Uncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetRxcrbfr_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxcrbfr_1Uncorrectable"))
	return nil
}

// SetRxcrbfr_1Correctable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetRxcrbfr_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxcrbfr_1Correctable"))
	return nil
}

// SetRxcrbfr_2Uncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetRxcrbfr_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxcrbfr_2Uncorrectable"))
	return nil
}

// SetRxcrbfr_2Correctable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetRxcrbfr_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxcrbfr_2Correctable"))
	return nil
}

// SetRxcrbfr_3Uncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetRxcrbfr_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxcrbfr_3Uncorrectable"))
	return nil
}

// SetRxcrbfr_3Correctable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetRxcrbfr_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxcrbfr_3Correctable"))
	return nil
}

// SetRxinfoUncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetRxinfoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxinfoUncorrectable"))
	return nil
}

// SetRxinfoCorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetRxinfoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxinfoCorrectable"))
	return nil
}

// SetCplstUncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetCplstUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CplstUncorrectable"))
	return nil
}

// SetCplstCorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetCplstCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CplstCorrectable"))
	return nil
}

// SetRomaskUncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetRomaskUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RomaskUncorrectable"))
	return nil
}

// SetRomaskCorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetRomaskCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RomaskCorrectable"))
	return nil
}

// SetPmrUncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetPmrUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PmrUncorrectable"))
	return nil
}

// SetPmrCorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetPmrCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PmrCorrectable"))
	return nil
}

// SetPrtUncorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetPrtUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrtUncorrectable"))
	return nil
}

// SetPrtCorrectable sets cunter in shared memory
func (mtr *PxbpxbinttgteccMetrics) SetPrtCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrtCorrectable"))
	return nil
}

// PxbpxbinttgteccMetricsIterator is the iterator object
type PxbpxbinttgteccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PxbpxbinttgteccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PxbpxbinttgteccMetricsIterator) Next() *PxbpxbinttgteccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PxbpxbinttgteccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PxbpxbinttgteccMetricsIterator) Find(key uint64) (*PxbpxbinttgteccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PxbpxbinttgteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PxbpxbinttgteccMetricsIterator) Create(key uint64) (*PxbpxbinttgteccMetrics, error) {
	tmtr := &PxbpxbinttgteccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PxbpxbinttgteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PxbpxbinttgteccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PxbpxbinttgteccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPxbpxbinttgteccMetricsIterator returns an iterator
func NewPxbpxbinttgteccMetricsIterator() (*PxbpxbinttgteccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PxbpxbinttgteccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PxbpxbinttgteccMetricsIterator{iter: iter}, nil
}

type PxbpxbintitreccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PcihdrtUncorrectable metrics.Counter

	PcihdrtCorrectable metrics.Counter

	PortmapUncorrectable metrics.Counter

	PortmapCorrectable metrics.Counter

	WrhdrUncorrectable metrics.Counter

	WrhdrCorrectable metrics.Counter

	RdhdrUncorrectable metrics.Counter

	RdhdrCorrectable metrics.Counter

	Wrbfr_0Uncorrectable metrics.Counter

	Wrbfr_0Correctable metrics.Counter

	Wrbfr_1Uncorrectable metrics.Counter

	Wrbfr_1Correctable metrics.Counter

	Wrbfr_2Uncorrectable metrics.Counter

	Wrbfr_2Correctable metrics.Counter

	Wrbfr_3Uncorrectable metrics.Counter

	Wrbfr_3Correctable metrics.Counter

	CplbfrUncorrectable metrics.Counter

	CplbfrCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PxbpxbintitreccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PxbpxbintitreccMetrics) Size() int {
	sz := 0

	sz += mtr.PcihdrtUncorrectable.Size()

	sz += mtr.PcihdrtCorrectable.Size()

	sz += mtr.PortmapUncorrectable.Size()

	sz += mtr.PortmapCorrectable.Size()

	sz += mtr.WrhdrUncorrectable.Size()

	sz += mtr.WrhdrCorrectable.Size()

	sz += mtr.RdhdrUncorrectable.Size()

	sz += mtr.RdhdrCorrectable.Size()

	sz += mtr.Wrbfr_0Uncorrectable.Size()

	sz += mtr.Wrbfr_0Correctable.Size()

	sz += mtr.Wrbfr_1Uncorrectable.Size()

	sz += mtr.Wrbfr_1Correctable.Size()

	sz += mtr.Wrbfr_2Uncorrectable.Size()

	sz += mtr.Wrbfr_2Correctable.Size()

	sz += mtr.Wrbfr_3Uncorrectable.Size()

	sz += mtr.Wrbfr_3Correctable.Size()

	sz += mtr.CplbfrUncorrectable.Size()

	sz += mtr.CplbfrCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PxbpxbintitreccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PcihdrtUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PcihdrtUncorrectable.Size()

	mtr.PcihdrtCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PcihdrtCorrectable.Size()

	mtr.PortmapUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PortmapUncorrectable.Size()

	mtr.PortmapCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PortmapCorrectable.Size()

	mtr.WrhdrUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.WrhdrUncorrectable.Size()

	mtr.WrhdrCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.WrhdrCorrectable.Size()

	mtr.RdhdrUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RdhdrUncorrectable.Size()

	mtr.RdhdrCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RdhdrCorrectable.Size()

	mtr.Wrbfr_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrbfr_0Uncorrectable.Size()

	mtr.Wrbfr_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrbfr_0Correctable.Size()

	mtr.Wrbfr_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrbfr_1Uncorrectable.Size()

	mtr.Wrbfr_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrbfr_1Correctable.Size()

	mtr.Wrbfr_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrbfr_2Uncorrectable.Size()

	mtr.Wrbfr_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrbfr_2Correctable.Size()

	mtr.Wrbfr_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrbfr_3Uncorrectable.Size()

	mtr.Wrbfr_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrbfr_3Correctable.Size()

	mtr.CplbfrUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.CplbfrUncorrectable.Size()

	mtr.CplbfrCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.CplbfrCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PxbpxbintitreccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PcihdrtUncorrectable" {
		return offset
	}
	offset += mtr.PcihdrtUncorrectable.Size()

	if fldName == "PcihdrtCorrectable" {
		return offset
	}
	offset += mtr.PcihdrtCorrectable.Size()

	if fldName == "PortmapUncorrectable" {
		return offset
	}
	offset += mtr.PortmapUncorrectable.Size()

	if fldName == "PortmapCorrectable" {
		return offset
	}
	offset += mtr.PortmapCorrectable.Size()

	if fldName == "WrhdrUncorrectable" {
		return offset
	}
	offset += mtr.WrhdrUncorrectable.Size()

	if fldName == "WrhdrCorrectable" {
		return offset
	}
	offset += mtr.WrhdrCorrectable.Size()

	if fldName == "RdhdrUncorrectable" {
		return offset
	}
	offset += mtr.RdhdrUncorrectable.Size()

	if fldName == "RdhdrCorrectable" {
		return offset
	}
	offset += mtr.RdhdrCorrectable.Size()

	if fldName == "Wrbfr_0Uncorrectable" {
		return offset
	}
	offset += mtr.Wrbfr_0Uncorrectable.Size()

	if fldName == "Wrbfr_0Correctable" {
		return offset
	}
	offset += mtr.Wrbfr_0Correctable.Size()

	if fldName == "Wrbfr_1Uncorrectable" {
		return offset
	}
	offset += mtr.Wrbfr_1Uncorrectable.Size()

	if fldName == "Wrbfr_1Correctable" {
		return offset
	}
	offset += mtr.Wrbfr_1Correctable.Size()

	if fldName == "Wrbfr_2Uncorrectable" {
		return offset
	}
	offset += mtr.Wrbfr_2Uncorrectable.Size()

	if fldName == "Wrbfr_2Correctable" {
		return offset
	}
	offset += mtr.Wrbfr_2Correctable.Size()

	if fldName == "Wrbfr_3Uncorrectable" {
		return offset
	}
	offset += mtr.Wrbfr_3Uncorrectable.Size()

	if fldName == "Wrbfr_3Correctable" {
		return offset
	}
	offset += mtr.Wrbfr_3Correctable.Size()

	if fldName == "CplbfrUncorrectable" {
		return offset
	}
	offset += mtr.CplbfrUncorrectable.Size()

	if fldName == "CplbfrCorrectable" {
		return offset
	}
	offset += mtr.CplbfrCorrectable.Size()

	return offset
}

// SetPcihdrtUncorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetPcihdrtUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PcihdrtUncorrectable"))
	return nil
}

// SetPcihdrtCorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetPcihdrtCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PcihdrtCorrectable"))
	return nil
}

// SetPortmapUncorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetPortmapUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PortmapUncorrectable"))
	return nil
}

// SetPortmapCorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetPortmapCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PortmapCorrectable"))
	return nil
}

// SetWrhdrUncorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetWrhdrUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrhdrUncorrectable"))
	return nil
}

// SetWrhdrCorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetWrhdrCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrhdrCorrectable"))
	return nil
}

// SetRdhdrUncorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetRdhdrUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdhdrUncorrectable"))
	return nil
}

// SetRdhdrCorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetRdhdrCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdhdrCorrectable"))
	return nil
}

// SetWrbfr_0Uncorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetWrbfr_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrbfr_0Uncorrectable"))
	return nil
}

// SetWrbfr_0Correctable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetWrbfr_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrbfr_0Correctable"))
	return nil
}

// SetWrbfr_1Uncorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetWrbfr_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrbfr_1Uncorrectable"))
	return nil
}

// SetWrbfr_1Correctable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetWrbfr_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrbfr_1Correctable"))
	return nil
}

// SetWrbfr_2Uncorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetWrbfr_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrbfr_2Uncorrectable"))
	return nil
}

// SetWrbfr_2Correctable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetWrbfr_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrbfr_2Correctable"))
	return nil
}

// SetWrbfr_3Uncorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetWrbfr_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrbfr_3Uncorrectable"))
	return nil
}

// SetWrbfr_3Correctable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetWrbfr_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrbfr_3Correctable"))
	return nil
}

// SetCplbfrUncorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetCplbfrUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CplbfrUncorrectable"))
	return nil
}

// SetCplbfrCorrectable sets cunter in shared memory
func (mtr *PxbpxbintitreccMetrics) SetCplbfrCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CplbfrCorrectable"))
	return nil
}

// PxbpxbintitreccMetricsIterator is the iterator object
type PxbpxbintitreccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PxbpxbintitreccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PxbpxbintitreccMetricsIterator) Next() *PxbpxbintitreccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PxbpxbintitreccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PxbpxbintitreccMetricsIterator) Find(key uint64) (*PxbpxbintitreccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PxbpxbintitreccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PxbpxbintitreccMetricsIterator) Create(key uint64) (*PxbpxbintitreccMetrics, error) {
	tmtr := &PxbpxbintitreccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PxbpxbintitreccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PxbpxbintitreccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PxbpxbintitreccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPxbpxbintitreccMetricsIterator returns an iterator
func NewPxbpxbintitreccMetricsIterator() (*PxbpxbintitreccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PxbpxbintitreccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PxbpxbintitreccMetricsIterator{iter: iter}, nil
}

type PxbpxbinterrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CplEcrc metrics.Counter

	CplRxbufErr metrics.Counter

	CplStat metrics.Counter

	CplTimeout metrics.Counter

	CplLen metrics.Counter

	CplPoison metrics.Counter

	CplLowaddr metrics.Counter

	TgtIndRspUnexp metrics.Counter

	TgtAxiRspUnexp metrics.Counter

	TgtAxiRspErr metrics.Counter

	TgtReq_4KErr metrics.Counter

	ItrReqBdfmiss metrics.Counter

	ItrReqAxiUnsupp metrics.Counter

	ItrReqMsgAlignErr metrics.Counter

	ItrBusMasterDis metrics.Counter

	ItrAtomicSeqErr metrics.Counter

	ItrAtomicResourceErr metrics.Counter

	TgtRmwAxiResperr metrics.Counter

	TgtIndNotifyAxiResperr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PxbpxbinterrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PxbpxbinterrMetrics) Size() int {
	sz := 0

	sz += mtr.CplEcrc.Size()

	sz += mtr.CplRxbufErr.Size()

	sz += mtr.CplStat.Size()

	sz += mtr.CplTimeout.Size()

	sz += mtr.CplLen.Size()

	sz += mtr.CplPoison.Size()

	sz += mtr.CplLowaddr.Size()

	sz += mtr.TgtIndRspUnexp.Size()

	sz += mtr.TgtAxiRspUnexp.Size()

	sz += mtr.TgtAxiRspErr.Size()

	sz += mtr.TgtReq_4KErr.Size()

	sz += mtr.ItrReqBdfmiss.Size()

	sz += mtr.ItrReqAxiUnsupp.Size()

	sz += mtr.ItrReqMsgAlignErr.Size()

	sz += mtr.ItrBusMasterDis.Size()

	sz += mtr.ItrAtomicSeqErr.Size()

	sz += mtr.ItrAtomicResourceErr.Size()

	sz += mtr.TgtRmwAxiResperr.Size()

	sz += mtr.TgtIndNotifyAxiResperr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PxbpxbinterrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CplEcrc = mtr.metrics.GetCounter(offset)
	offset += mtr.CplEcrc.Size()

	mtr.CplRxbufErr = mtr.metrics.GetCounter(offset)
	offset += mtr.CplRxbufErr.Size()

	mtr.CplStat = mtr.metrics.GetCounter(offset)
	offset += mtr.CplStat.Size()

	mtr.CplTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.CplTimeout.Size()

	mtr.CplLen = mtr.metrics.GetCounter(offset)
	offset += mtr.CplLen.Size()

	mtr.CplPoison = mtr.metrics.GetCounter(offset)
	offset += mtr.CplPoison.Size()

	mtr.CplLowaddr = mtr.metrics.GetCounter(offset)
	offset += mtr.CplLowaddr.Size()

	mtr.TgtIndRspUnexp = mtr.metrics.GetCounter(offset)
	offset += mtr.TgtIndRspUnexp.Size()

	mtr.TgtAxiRspUnexp = mtr.metrics.GetCounter(offset)
	offset += mtr.TgtAxiRspUnexp.Size()

	mtr.TgtAxiRspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.TgtAxiRspErr.Size()

	mtr.TgtReq_4KErr = mtr.metrics.GetCounter(offset)
	offset += mtr.TgtReq_4KErr.Size()

	mtr.ItrReqBdfmiss = mtr.metrics.GetCounter(offset)
	offset += mtr.ItrReqBdfmiss.Size()

	mtr.ItrReqAxiUnsupp = mtr.metrics.GetCounter(offset)
	offset += mtr.ItrReqAxiUnsupp.Size()

	mtr.ItrReqMsgAlignErr = mtr.metrics.GetCounter(offset)
	offset += mtr.ItrReqMsgAlignErr.Size()

	mtr.ItrBusMasterDis = mtr.metrics.GetCounter(offset)
	offset += mtr.ItrBusMasterDis.Size()

	mtr.ItrAtomicSeqErr = mtr.metrics.GetCounter(offset)
	offset += mtr.ItrAtomicSeqErr.Size()

	mtr.ItrAtomicResourceErr = mtr.metrics.GetCounter(offset)
	offset += mtr.ItrAtomicResourceErr.Size()

	mtr.TgtRmwAxiResperr = mtr.metrics.GetCounter(offset)
	offset += mtr.TgtRmwAxiResperr.Size()

	mtr.TgtIndNotifyAxiResperr = mtr.metrics.GetCounter(offset)
	offset += mtr.TgtIndNotifyAxiResperr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PxbpxbinterrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CplEcrc" {
		return offset
	}
	offset += mtr.CplEcrc.Size()

	if fldName == "CplRxbufErr" {
		return offset
	}
	offset += mtr.CplRxbufErr.Size()

	if fldName == "CplStat" {
		return offset
	}
	offset += mtr.CplStat.Size()

	if fldName == "CplTimeout" {
		return offset
	}
	offset += mtr.CplTimeout.Size()

	if fldName == "CplLen" {
		return offset
	}
	offset += mtr.CplLen.Size()

	if fldName == "CplPoison" {
		return offset
	}
	offset += mtr.CplPoison.Size()

	if fldName == "CplLowaddr" {
		return offset
	}
	offset += mtr.CplLowaddr.Size()

	if fldName == "TgtIndRspUnexp" {
		return offset
	}
	offset += mtr.TgtIndRspUnexp.Size()

	if fldName == "TgtAxiRspUnexp" {
		return offset
	}
	offset += mtr.TgtAxiRspUnexp.Size()

	if fldName == "TgtAxiRspErr" {
		return offset
	}
	offset += mtr.TgtAxiRspErr.Size()

	if fldName == "TgtReq_4KErr" {
		return offset
	}
	offset += mtr.TgtReq_4KErr.Size()

	if fldName == "ItrReqBdfmiss" {
		return offset
	}
	offset += mtr.ItrReqBdfmiss.Size()

	if fldName == "ItrReqAxiUnsupp" {
		return offset
	}
	offset += mtr.ItrReqAxiUnsupp.Size()

	if fldName == "ItrReqMsgAlignErr" {
		return offset
	}
	offset += mtr.ItrReqMsgAlignErr.Size()

	if fldName == "ItrBusMasterDis" {
		return offset
	}
	offset += mtr.ItrBusMasterDis.Size()

	if fldName == "ItrAtomicSeqErr" {
		return offset
	}
	offset += mtr.ItrAtomicSeqErr.Size()

	if fldName == "ItrAtomicResourceErr" {
		return offset
	}
	offset += mtr.ItrAtomicResourceErr.Size()

	if fldName == "TgtRmwAxiResperr" {
		return offset
	}
	offset += mtr.TgtRmwAxiResperr.Size()

	if fldName == "TgtIndNotifyAxiResperr" {
		return offset
	}
	offset += mtr.TgtIndNotifyAxiResperr.Size()

	return offset
}

// SetCplEcrc sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetCplEcrc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CplEcrc"))
	return nil
}

// SetCplRxbufErr sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetCplRxbufErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CplRxbufErr"))
	return nil
}

// SetCplStat sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetCplStat(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CplStat"))
	return nil
}

// SetCplTimeout sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetCplTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CplTimeout"))
	return nil
}

// SetCplLen sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetCplLen(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CplLen"))
	return nil
}

// SetCplPoison sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetCplPoison(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CplPoison"))
	return nil
}

// SetCplLowaddr sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetCplLowaddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CplLowaddr"))
	return nil
}

// SetTgtIndRspUnexp sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetTgtIndRspUnexp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TgtIndRspUnexp"))
	return nil
}

// SetTgtAxiRspUnexp sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetTgtAxiRspUnexp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TgtAxiRspUnexp"))
	return nil
}

// SetTgtAxiRspErr sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetTgtAxiRspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TgtAxiRspErr"))
	return nil
}

// SetTgtReq_4KErr sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetTgtReq_4KErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TgtReq_4KErr"))
	return nil
}

// SetItrReqBdfmiss sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetItrReqBdfmiss(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ItrReqBdfmiss"))
	return nil
}

// SetItrReqAxiUnsupp sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetItrReqAxiUnsupp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ItrReqAxiUnsupp"))
	return nil
}

// SetItrReqMsgAlignErr sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetItrReqMsgAlignErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ItrReqMsgAlignErr"))
	return nil
}

// SetItrBusMasterDis sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetItrBusMasterDis(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ItrBusMasterDis"))
	return nil
}

// SetItrAtomicSeqErr sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetItrAtomicSeqErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ItrAtomicSeqErr"))
	return nil
}

// SetItrAtomicResourceErr sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetItrAtomicResourceErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ItrAtomicResourceErr"))
	return nil
}

// SetTgtRmwAxiResperr sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetTgtRmwAxiResperr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TgtRmwAxiResperr"))
	return nil
}

// SetTgtIndNotifyAxiResperr sets cunter in shared memory
func (mtr *PxbpxbinterrMetrics) SetTgtIndNotifyAxiResperr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TgtIndNotifyAxiResperr"))
	return nil
}

// PxbpxbinterrMetricsIterator is the iterator object
type PxbpxbinterrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PxbpxbinterrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PxbpxbinterrMetricsIterator) Next() *PxbpxbinterrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PxbpxbinterrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PxbpxbinterrMetricsIterator) Find(key uint64) (*PxbpxbinterrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PxbpxbinterrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PxbpxbinterrMetricsIterator) Create(key uint64) (*PxbpxbinterrMetrics, error) {
	tmtr := &PxbpxbinterrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PxbpxbinterrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PxbpxbinterrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PxbpxbinterrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPxbpxbinterrMetricsIterator returns an iterator
func NewPxbpxbinterrMetricsIterator() (*PxbpxbinterrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PxbpxbinterrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PxbpxbinterrMetricsIterator{iter: iter}, nil
}

type Sgete0interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgete0interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgete0interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

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
func (mtr *Sgete0interrMetrics) Unmarshal() error {
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
func (mtr *Sgete0interrMetrics) getOffset(fldName string) int {
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
func (mtr *Sgete0interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Sgete0interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Sgete0interrMetricsIterator is the iterator object
type Sgete0interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgete0interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgete0interrMetricsIterator) Next() *Sgete0interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgete0interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgete0interrMetricsIterator) Find(key uint64) (*Sgete0interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgete0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgete0interrMetricsIterator) Create(key uint64) (*Sgete0interrMetrics, error) {
	tmtr := &Sgete0interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgete0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgete0interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgete0interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgete0interrMetricsIterator returns an iterator
func NewSgete0interrMetricsIterator() (*Sgete0interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgete0interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgete0interrMetricsIterator{iter: iter}, nil
}

type Sgete1interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgete1interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgete1interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

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
func (mtr *Sgete1interrMetrics) Unmarshal() error {
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
func (mtr *Sgete1interrMetrics) getOffset(fldName string) int {
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
func (mtr *Sgete1interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Sgete1interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Sgete1interrMetricsIterator is the iterator object
type Sgete1interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgete1interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgete1interrMetricsIterator) Next() *Sgete1interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgete1interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgete1interrMetricsIterator) Find(key uint64) (*Sgete1interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgete1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgete1interrMetricsIterator) Create(key uint64) (*Sgete1interrMetrics, error) {
	tmtr := &Sgete1interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgete1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgete1interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgete1interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgete1interrMetricsIterator returns an iterator
func NewSgete1interrMetricsIterator() (*Sgete1interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgete1interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgete1interrMetricsIterator{iter: iter}, nil
}

type Sgete2interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgete2interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgete2interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

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
func (mtr *Sgete2interrMetrics) Unmarshal() error {
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
func (mtr *Sgete2interrMetrics) getOffset(fldName string) int {
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
func (mtr *Sgete2interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Sgete2interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Sgete2interrMetricsIterator is the iterator object
type Sgete2interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgete2interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgete2interrMetricsIterator) Next() *Sgete2interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgete2interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgete2interrMetricsIterator) Find(key uint64) (*Sgete2interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgete2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgete2interrMetricsIterator) Create(key uint64) (*Sgete2interrMetrics, error) {
	tmtr := &Sgete2interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgete2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgete2interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgete2interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgete2interrMetricsIterator returns an iterator
func NewSgete2interrMetricsIterator() (*Sgete2interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgete2interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgete2interrMetricsIterator{iter: iter}, nil
}

type Sgete3interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgete3interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgete3interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

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
func (mtr *Sgete3interrMetrics) Unmarshal() error {
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
func (mtr *Sgete3interrMetrics) getOffset(fldName string) int {
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
func (mtr *Sgete3interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Sgete3interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Sgete3interrMetricsIterator is the iterator object
type Sgete3interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgete3interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgete3interrMetricsIterator) Next() *Sgete3interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgete3interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgete3interrMetricsIterator) Find(key uint64) (*Sgete3interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgete3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgete3interrMetricsIterator) Create(key uint64) (*Sgete3interrMetrics, error) {
	tmtr := &Sgete3interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgete3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgete3interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgete3interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgete3interrMetricsIterator returns an iterator
func NewSgete3interrMetricsIterator() (*Sgete3interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgete3interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgete3interrMetricsIterator{iter: iter}, nil
}

type Sgete4interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgete4interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgete4interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

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
func (mtr *Sgete4interrMetrics) Unmarshal() error {
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
func (mtr *Sgete4interrMetrics) getOffset(fldName string) int {
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
func (mtr *Sgete4interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Sgete4interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Sgete4interrMetricsIterator is the iterator object
type Sgete4interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgete4interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgete4interrMetricsIterator) Next() *Sgete4interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgete4interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgete4interrMetricsIterator) Find(key uint64) (*Sgete4interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgete4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgete4interrMetricsIterator) Create(key uint64) (*Sgete4interrMetrics, error) {
	tmtr := &Sgete4interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgete4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgete4interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgete4interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgete4interrMetricsIterator returns an iterator
func NewSgete4interrMetricsIterator() (*Sgete4interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgete4interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgete4interrMetricsIterator{iter: iter}, nil
}

type Sgete5interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgete5interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgete5interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

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
func (mtr *Sgete5interrMetrics) Unmarshal() error {
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
func (mtr *Sgete5interrMetrics) getOffset(fldName string) int {
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
func (mtr *Sgete5interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Sgete5interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Sgete5interrMetricsIterator is the iterator object
type Sgete5interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgete5interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgete5interrMetricsIterator) Next() *Sgete5interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgete5interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgete5interrMetricsIterator) Find(key uint64) (*Sgete5interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgete5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgete5interrMetricsIterator) Create(key uint64) (*Sgete5interrMetrics, error) {
	tmtr := &Sgete5interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgete5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgete5interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgete5interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgete5interrMetricsIterator returns an iterator
func NewSgete5interrMetricsIterator() (*Sgete5interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgete5interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgete5interrMetricsIterator{iter: iter}, nil
}

type Sgempu0interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgempu0interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgempu0interrMetrics) Size() int {
	sz := 0

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
func (mtr *Sgempu0interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgempu0interrMetrics) getOffset(fldName string) int {
	var offset int

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

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Sgempu0interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Sgempu0interrMetricsIterator is the iterator object
type Sgempu0interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgempu0interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgempu0interrMetricsIterator) Next() *Sgempu0interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgempu0interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgempu0interrMetricsIterator) Find(key uint64) (*Sgempu0interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgempu0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgempu0interrMetricsIterator) Create(key uint64) (*Sgempu0interrMetrics, error) {
	tmtr := &Sgempu0interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgempu0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgempu0interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgempu0interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgempu0interrMetricsIterator returns an iterator
func NewSgempu0interrMetricsIterator() (*Sgempu0interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgempu0interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgempu0interrMetricsIterator{iter: iter}, nil
}

type Sgempu0intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgempu0intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgempu0intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Sgempu0intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgempu0intinfoMetrics) getOffset(fldName string) int {
	var offset int

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

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Sgempu0intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Sgempu0intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Sgempu0intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Sgempu0intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Sgempu0intinfoMetricsIterator is the iterator object
type Sgempu0intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgempu0intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgempu0intinfoMetricsIterator) Next() *Sgempu0intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgempu0intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgempu0intinfoMetricsIterator) Find(key uint64) (*Sgempu0intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgempu0intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgempu0intinfoMetricsIterator) Create(key uint64) (*Sgempu0intinfoMetrics, error) {
	tmtr := &Sgempu0intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgempu0intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgempu0intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgempu0intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgempu0intinfoMetricsIterator returns an iterator
func NewSgempu0intinfoMetricsIterator() (*Sgempu0intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgempu0intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgempu0intinfoMetricsIterator{iter: iter}, nil
}

type Sgempu1interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgempu1interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgempu1interrMetrics) Size() int {
	sz := 0

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
func (mtr *Sgempu1interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgempu1interrMetrics) getOffset(fldName string) int {
	var offset int

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

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Sgempu1interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Sgempu1interrMetricsIterator is the iterator object
type Sgempu1interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgempu1interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgempu1interrMetricsIterator) Next() *Sgempu1interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgempu1interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgempu1interrMetricsIterator) Find(key uint64) (*Sgempu1interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgempu1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgempu1interrMetricsIterator) Create(key uint64) (*Sgempu1interrMetrics, error) {
	tmtr := &Sgempu1interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgempu1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgempu1interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgempu1interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgempu1interrMetricsIterator returns an iterator
func NewSgempu1interrMetricsIterator() (*Sgempu1interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgempu1interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgempu1interrMetricsIterator{iter: iter}, nil
}

type Sgempu1intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgempu1intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgempu1intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Sgempu1intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgempu1intinfoMetrics) getOffset(fldName string) int {
	var offset int

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

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Sgempu1intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Sgempu1intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Sgempu1intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Sgempu1intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Sgempu1intinfoMetricsIterator is the iterator object
type Sgempu1intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgempu1intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgempu1intinfoMetricsIterator) Next() *Sgempu1intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgempu1intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgempu1intinfoMetricsIterator) Find(key uint64) (*Sgempu1intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgempu1intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgempu1intinfoMetricsIterator) Create(key uint64) (*Sgempu1intinfoMetrics, error) {
	tmtr := &Sgempu1intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgempu1intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgempu1intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgempu1intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgempu1intinfoMetricsIterator returns an iterator
func NewSgempu1intinfoMetricsIterator() (*Sgempu1intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgempu1intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgempu1intinfoMetricsIterator{iter: iter}, nil
}

type Sgempu2interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgempu2interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgempu2interrMetrics) Size() int {
	sz := 0

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
func (mtr *Sgempu2interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgempu2interrMetrics) getOffset(fldName string) int {
	var offset int

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

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Sgempu2interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Sgempu2interrMetricsIterator is the iterator object
type Sgempu2interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgempu2interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgempu2interrMetricsIterator) Next() *Sgempu2interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgempu2interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgempu2interrMetricsIterator) Find(key uint64) (*Sgempu2interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgempu2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgempu2interrMetricsIterator) Create(key uint64) (*Sgempu2interrMetrics, error) {
	tmtr := &Sgempu2interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgempu2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgempu2interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgempu2interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgempu2interrMetricsIterator returns an iterator
func NewSgempu2interrMetricsIterator() (*Sgempu2interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgempu2interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgempu2interrMetricsIterator{iter: iter}, nil
}

type Sgempu2intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgempu2intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgempu2intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Sgempu2intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgempu2intinfoMetrics) getOffset(fldName string) int {
	var offset int

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

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Sgempu2intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Sgempu2intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Sgempu2intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Sgempu2intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Sgempu2intinfoMetricsIterator is the iterator object
type Sgempu2intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgempu2intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgempu2intinfoMetricsIterator) Next() *Sgempu2intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgempu2intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgempu2intinfoMetricsIterator) Find(key uint64) (*Sgempu2intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgempu2intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgempu2intinfoMetricsIterator) Create(key uint64) (*Sgempu2intinfoMetrics, error) {
	tmtr := &Sgempu2intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgempu2intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgempu2intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgempu2intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgempu2intinfoMetricsIterator returns an iterator
func NewSgempu2intinfoMetricsIterator() (*Sgempu2intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgempu2intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgempu2intinfoMetricsIterator{iter: iter}, nil
}

type Sgempu3interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgempu3interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgempu3interrMetrics) Size() int {
	sz := 0

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
func (mtr *Sgempu3interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgempu3interrMetrics) getOffset(fldName string) int {
	var offset int

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

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Sgempu3interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Sgempu3interrMetricsIterator is the iterator object
type Sgempu3interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgempu3interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgempu3interrMetricsIterator) Next() *Sgempu3interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgempu3interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgempu3interrMetricsIterator) Find(key uint64) (*Sgempu3interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgempu3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgempu3interrMetricsIterator) Create(key uint64) (*Sgempu3interrMetrics, error) {
	tmtr := &Sgempu3interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgempu3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgempu3interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgempu3interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgempu3interrMetricsIterator returns an iterator
func NewSgempu3interrMetricsIterator() (*Sgempu3interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgempu3interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgempu3interrMetricsIterator{iter: iter}, nil
}

type Sgempu3intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgempu3intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgempu3intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Sgempu3intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgempu3intinfoMetrics) getOffset(fldName string) int {
	var offset int

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

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Sgempu3intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Sgempu3intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Sgempu3intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Sgempu3intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Sgempu3intinfoMetricsIterator is the iterator object
type Sgempu3intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgempu3intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgempu3intinfoMetricsIterator) Next() *Sgempu3intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgempu3intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgempu3intinfoMetricsIterator) Find(key uint64) (*Sgempu3intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgempu3intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgempu3intinfoMetricsIterator) Create(key uint64) (*Sgempu3intinfoMetrics, error) {
	tmtr := &Sgempu3intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgempu3intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgempu3intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgempu3intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgempu3intinfoMetricsIterator returns an iterator
func NewSgempu3intinfoMetricsIterator() (*Sgempu3intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgempu3intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgempu3intinfoMetricsIterator{iter: iter}, nil
}

type Sgempu4interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgempu4interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgempu4interrMetrics) Size() int {
	sz := 0

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
func (mtr *Sgempu4interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgempu4interrMetrics) getOffset(fldName string) int {
	var offset int

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

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Sgempu4interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Sgempu4interrMetricsIterator is the iterator object
type Sgempu4interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgempu4interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgempu4interrMetricsIterator) Next() *Sgempu4interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgempu4interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgempu4interrMetricsIterator) Find(key uint64) (*Sgempu4interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgempu4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgempu4interrMetricsIterator) Create(key uint64) (*Sgempu4interrMetrics, error) {
	tmtr := &Sgempu4interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgempu4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgempu4interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgempu4interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgempu4interrMetricsIterator returns an iterator
func NewSgempu4interrMetricsIterator() (*Sgempu4interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgempu4interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgempu4interrMetricsIterator{iter: iter}, nil
}

type Sgempu4intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgempu4intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgempu4intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Sgempu4intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgempu4intinfoMetrics) getOffset(fldName string) int {
	var offset int

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

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Sgempu4intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Sgempu4intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Sgempu4intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Sgempu4intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Sgempu4intinfoMetricsIterator is the iterator object
type Sgempu4intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgempu4intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgempu4intinfoMetricsIterator) Next() *Sgempu4intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgempu4intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgempu4intinfoMetricsIterator) Find(key uint64) (*Sgempu4intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgempu4intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgempu4intinfoMetricsIterator) Create(key uint64) (*Sgempu4intinfoMetrics, error) {
	tmtr := &Sgempu4intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgempu4intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgempu4intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgempu4intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgempu4intinfoMetricsIterator returns an iterator
func NewSgempu4intinfoMetricsIterator() (*Sgempu4intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgempu4intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgempu4intinfoMetricsIterator{iter: iter}, nil
}

type Sgempu5interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgempu5interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgempu5interrMetrics) Size() int {
	sz := 0

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
func (mtr *Sgempu5interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgempu5interrMetrics) getOffset(fldName string) int {
	var offset int

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

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Sgempu5interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Sgempu5interrMetricsIterator is the iterator object
type Sgempu5interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgempu5interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgempu5interrMetricsIterator) Next() *Sgempu5interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgempu5interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgempu5interrMetricsIterator) Find(key uint64) (*Sgempu5interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgempu5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgempu5interrMetricsIterator) Create(key uint64) (*Sgempu5interrMetrics, error) {
	tmtr := &Sgempu5interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgempu5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgempu5interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgempu5interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgempu5interrMetricsIterator returns an iterator
func NewSgempu5interrMetricsIterator() (*Sgempu5interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgempu5interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgempu5interrMetricsIterator{iter: iter}, nil
}

type Sgempu5intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Sgempu5intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Sgempu5intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Sgempu5intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *Sgempu5intinfoMetrics) getOffset(fldName string) int {
	var offset int

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

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Sgempu5intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Sgempu5intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Sgempu5intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Sgempu5intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Sgempu5intinfoMetricsIterator is the iterator object
type Sgempu5intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Sgempu5intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Sgempu5intinfoMetricsIterator) Next() *Sgempu5intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Sgempu5intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Sgempu5intinfoMetricsIterator) Find(key uint64) (*Sgempu5intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Sgempu5intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Sgempu5intinfoMetricsIterator) Create(key uint64) (*Sgempu5intinfoMetrics, error) {
	tmtr := &Sgempu5intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Sgempu5intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Sgempu5intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Sgempu5intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewSgempu5intinfoMetricsIterator returns an iterator
func NewSgempu5intinfoMetricsIterator() (*Sgempu5intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Sgempu5intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Sgempu5intinfoMetricsIterator{iter: iter}, nil
}

type PrprpspinterrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PbPbusErr metrics.Counter

	PrPbusErr metrics.Counter

	PrResubPbusErr metrics.Counter

	PspCsrReadAccessErr metrics.Counter

	PspCsrWriteAccessErr metrics.Counter

	PspPhvLargerThanTwelveFlitsErr metrics.Counter

	PspPhvSopOffsetErr metrics.Counter

	PspPhvEopOffsetErr metrics.Counter

	MaxRecirCountErr metrics.Counter

	PbPbusDummyErr metrics.Counter

	PbPbusNopktErr metrics.Counter

	PbPbusMinErr metrics.Counter

	LbPhvAlmostFullTimeoutErr metrics.Counter

	LbPktAlmostFullTimeoutErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PrprpspinterrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PrprpspinterrMetrics) Size() int {
	sz := 0

	sz += mtr.PbPbusErr.Size()

	sz += mtr.PrPbusErr.Size()

	sz += mtr.PrResubPbusErr.Size()

	sz += mtr.PspCsrReadAccessErr.Size()

	sz += mtr.PspCsrWriteAccessErr.Size()

	sz += mtr.PspPhvLargerThanTwelveFlitsErr.Size()

	sz += mtr.PspPhvSopOffsetErr.Size()

	sz += mtr.PspPhvEopOffsetErr.Size()

	sz += mtr.MaxRecirCountErr.Size()

	sz += mtr.PbPbusDummyErr.Size()

	sz += mtr.PbPbusNopktErr.Size()

	sz += mtr.PbPbusMinErr.Size()

	sz += mtr.LbPhvAlmostFullTimeoutErr.Size()

	sz += mtr.LbPktAlmostFullTimeoutErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PrprpspinterrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PbPbusErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusErr.Size()

	mtr.PrPbusErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrPbusErr.Size()

	mtr.PrResubPbusErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrResubPbusErr.Size()

	mtr.PspCsrReadAccessErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PspCsrReadAccessErr.Size()

	mtr.PspCsrWriteAccessErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PspCsrWriteAccessErr.Size()

	mtr.PspPhvLargerThanTwelveFlitsErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PspPhvLargerThanTwelveFlitsErr.Size()

	mtr.PspPhvSopOffsetErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PspPhvSopOffsetErr.Size()

	mtr.PspPhvEopOffsetErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PspPhvEopOffsetErr.Size()

	mtr.MaxRecirCountErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxRecirCountErr.Size()

	mtr.PbPbusDummyErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusDummyErr.Size()

	mtr.PbPbusNopktErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusNopktErr.Size()

	mtr.PbPbusMinErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusMinErr.Size()

	mtr.LbPhvAlmostFullTimeoutErr = mtr.metrics.GetCounter(offset)
	offset += mtr.LbPhvAlmostFullTimeoutErr.Size()

	mtr.LbPktAlmostFullTimeoutErr = mtr.metrics.GetCounter(offset)
	offset += mtr.LbPktAlmostFullTimeoutErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PrprpspinterrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PbPbusErr" {
		return offset
	}
	offset += mtr.PbPbusErr.Size()

	if fldName == "PrPbusErr" {
		return offset
	}
	offset += mtr.PrPbusErr.Size()

	if fldName == "PrResubPbusErr" {
		return offset
	}
	offset += mtr.PrResubPbusErr.Size()

	if fldName == "PspCsrReadAccessErr" {
		return offset
	}
	offset += mtr.PspCsrReadAccessErr.Size()

	if fldName == "PspCsrWriteAccessErr" {
		return offset
	}
	offset += mtr.PspCsrWriteAccessErr.Size()

	if fldName == "PspPhvLargerThanTwelveFlitsErr" {
		return offset
	}
	offset += mtr.PspPhvLargerThanTwelveFlitsErr.Size()

	if fldName == "PspPhvSopOffsetErr" {
		return offset
	}
	offset += mtr.PspPhvSopOffsetErr.Size()

	if fldName == "PspPhvEopOffsetErr" {
		return offset
	}
	offset += mtr.PspPhvEopOffsetErr.Size()

	if fldName == "MaxRecirCountErr" {
		return offset
	}
	offset += mtr.MaxRecirCountErr.Size()

	if fldName == "PbPbusDummyErr" {
		return offset
	}
	offset += mtr.PbPbusDummyErr.Size()

	if fldName == "PbPbusNopktErr" {
		return offset
	}
	offset += mtr.PbPbusNopktErr.Size()

	if fldName == "PbPbusMinErr" {
		return offset
	}
	offset += mtr.PbPbusMinErr.Size()

	if fldName == "LbPhvAlmostFullTimeoutErr" {
		return offset
	}
	offset += mtr.LbPhvAlmostFullTimeoutErr.Size()

	if fldName == "LbPktAlmostFullTimeoutErr" {
		return offset
	}
	offset += mtr.LbPktAlmostFullTimeoutErr.Size()

	return offset
}

// SetPbPbusErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetPbPbusErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusErr"))
	return nil
}

// SetPrPbusErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetPrPbusErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrPbusErr"))
	return nil
}

// SetPrResubPbusErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetPrResubPbusErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrResubPbusErr"))
	return nil
}

// SetPspCsrReadAccessErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetPspCsrReadAccessErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PspCsrReadAccessErr"))
	return nil
}

// SetPspCsrWriteAccessErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetPspCsrWriteAccessErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PspCsrWriteAccessErr"))
	return nil
}

// SetPspPhvLargerThanTwelveFlitsErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetPspPhvLargerThanTwelveFlitsErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PspPhvLargerThanTwelveFlitsErr"))
	return nil
}

// SetPspPhvSopOffsetErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetPspPhvSopOffsetErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PspPhvSopOffsetErr"))
	return nil
}

// SetPspPhvEopOffsetErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetPspPhvEopOffsetErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PspPhvEopOffsetErr"))
	return nil
}

// SetMaxRecirCountErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetMaxRecirCountErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxRecirCountErr"))
	return nil
}

// SetPbPbusDummyErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetPbPbusDummyErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusDummyErr"))
	return nil
}

// SetPbPbusNopktErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetPbPbusNopktErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusNopktErr"))
	return nil
}

// SetPbPbusMinErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetPbPbusMinErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusMinErr"))
	return nil
}

// SetLbPhvAlmostFullTimeoutErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetLbPhvAlmostFullTimeoutErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LbPhvAlmostFullTimeoutErr"))
	return nil
}

// SetLbPktAlmostFullTimeoutErr sets cunter in shared memory
func (mtr *PrprpspinterrMetrics) SetLbPktAlmostFullTimeoutErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LbPktAlmostFullTimeoutErr"))
	return nil
}

// PrprpspinterrMetricsIterator is the iterator object
type PrprpspinterrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PrprpspinterrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PrprpspinterrMetricsIterator) Next() *PrprpspinterrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PrprpspinterrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PrprpspinterrMetricsIterator) Find(key uint64) (*PrprpspinterrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PrprpspinterrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PrprpspinterrMetricsIterator) Create(key uint64) (*PrprpspinterrMetrics, error) {
	tmtr := &PrprpspinterrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PrprpspinterrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PrprpspinterrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PrprpspinterrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPrprpspinterrMetricsIterator returns an iterator
func NewPrprpspinterrMetricsIterator() (*PrprpspinterrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PrprpspinterrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PrprpspinterrMetricsIterator{iter: iter}, nil
}

type PrprpspintfatalMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PbPbusSopErr metrics.Counter

	PbPbusEopErr metrics.Counter

	MaSopErr metrics.Counter

	MaEopErr metrics.Counter

	PrPbusSopErr metrics.Counter

	PrPbusEopErr metrics.Counter

	PrResubPbusSopErr metrics.Counter

	PrResubPbusEopErr metrics.Counter

	PrResubSopErr metrics.Counter

	PrResubEopErr metrics.Counter

	PbPbusFsm0NoDataErr metrics.Counter

	PbPbusFsm1NoDataErr metrics.Counter

	PbPbusFsm2NoDataErr metrics.Counter

	PbPbusFsm3NoDataErr metrics.Counter

	PbPbusFsm4NoDataErr metrics.Counter

	PbPbusFsm5PktTooSmallErr metrics.Counter

	LifIndTableRspErr metrics.Counter

	FlitCntOflowErr metrics.Counter

	LbPhvSopErr metrics.Counter

	DcPhvSopErr metrics.Counter

	LbPktSopErr metrics.Counter

	DcPktSopErr metrics.Counter

	SwPhvJabberErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PrprpspintfatalMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PrprpspintfatalMetrics) Size() int {
	sz := 0

	sz += mtr.PbPbusSopErr.Size()

	sz += mtr.PbPbusEopErr.Size()

	sz += mtr.MaSopErr.Size()

	sz += mtr.MaEopErr.Size()

	sz += mtr.PrPbusSopErr.Size()

	sz += mtr.PrPbusEopErr.Size()

	sz += mtr.PrResubPbusSopErr.Size()

	sz += mtr.PrResubPbusEopErr.Size()

	sz += mtr.PrResubSopErr.Size()

	sz += mtr.PrResubEopErr.Size()

	sz += mtr.PbPbusFsm0NoDataErr.Size()

	sz += mtr.PbPbusFsm1NoDataErr.Size()

	sz += mtr.PbPbusFsm2NoDataErr.Size()

	sz += mtr.PbPbusFsm3NoDataErr.Size()

	sz += mtr.PbPbusFsm4NoDataErr.Size()

	sz += mtr.PbPbusFsm5PktTooSmallErr.Size()

	sz += mtr.LifIndTableRspErr.Size()

	sz += mtr.FlitCntOflowErr.Size()

	sz += mtr.LbPhvSopErr.Size()

	sz += mtr.DcPhvSopErr.Size()

	sz += mtr.LbPktSopErr.Size()

	sz += mtr.DcPktSopErr.Size()

	sz += mtr.SwPhvJabberErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PrprpspintfatalMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PbPbusSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusSopErr.Size()

	mtr.PbPbusEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusEopErr.Size()

	mtr.MaSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaSopErr.Size()

	mtr.MaEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaEopErr.Size()

	mtr.PrPbusSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrPbusSopErr.Size()

	mtr.PrPbusEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrPbusEopErr.Size()

	mtr.PrResubPbusSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrResubPbusSopErr.Size()

	mtr.PrResubPbusEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrResubPbusEopErr.Size()

	mtr.PrResubSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrResubSopErr.Size()

	mtr.PrResubEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrResubEopErr.Size()

	mtr.PbPbusFsm0NoDataErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusFsm0NoDataErr.Size()

	mtr.PbPbusFsm1NoDataErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusFsm1NoDataErr.Size()

	mtr.PbPbusFsm2NoDataErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusFsm2NoDataErr.Size()

	mtr.PbPbusFsm3NoDataErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusFsm3NoDataErr.Size()

	mtr.PbPbusFsm4NoDataErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusFsm4NoDataErr.Size()

	mtr.PbPbusFsm5PktTooSmallErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusFsm5PktTooSmallErr.Size()

	mtr.LifIndTableRspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.LifIndTableRspErr.Size()

	mtr.FlitCntOflowErr = mtr.metrics.GetCounter(offset)
	offset += mtr.FlitCntOflowErr.Size()

	mtr.LbPhvSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.LbPhvSopErr.Size()

	mtr.DcPhvSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.DcPhvSopErr.Size()

	mtr.LbPktSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.LbPktSopErr.Size()

	mtr.DcPktSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.DcPktSopErr.Size()

	mtr.SwPhvJabberErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SwPhvJabberErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PrprpspintfatalMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PbPbusSopErr" {
		return offset
	}
	offset += mtr.PbPbusSopErr.Size()

	if fldName == "PbPbusEopErr" {
		return offset
	}
	offset += mtr.PbPbusEopErr.Size()

	if fldName == "MaSopErr" {
		return offset
	}
	offset += mtr.MaSopErr.Size()

	if fldName == "MaEopErr" {
		return offset
	}
	offset += mtr.MaEopErr.Size()

	if fldName == "PrPbusSopErr" {
		return offset
	}
	offset += mtr.PrPbusSopErr.Size()

	if fldName == "PrPbusEopErr" {
		return offset
	}
	offset += mtr.PrPbusEopErr.Size()

	if fldName == "PrResubPbusSopErr" {
		return offset
	}
	offset += mtr.PrResubPbusSopErr.Size()

	if fldName == "PrResubPbusEopErr" {
		return offset
	}
	offset += mtr.PrResubPbusEopErr.Size()

	if fldName == "PrResubSopErr" {
		return offset
	}
	offset += mtr.PrResubSopErr.Size()

	if fldName == "PrResubEopErr" {
		return offset
	}
	offset += mtr.PrResubEopErr.Size()

	if fldName == "PbPbusFsm0NoDataErr" {
		return offset
	}
	offset += mtr.PbPbusFsm0NoDataErr.Size()

	if fldName == "PbPbusFsm1NoDataErr" {
		return offset
	}
	offset += mtr.PbPbusFsm1NoDataErr.Size()

	if fldName == "PbPbusFsm2NoDataErr" {
		return offset
	}
	offset += mtr.PbPbusFsm2NoDataErr.Size()

	if fldName == "PbPbusFsm3NoDataErr" {
		return offset
	}
	offset += mtr.PbPbusFsm3NoDataErr.Size()

	if fldName == "PbPbusFsm4NoDataErr" {
		return offset
	}
	offset += mtr.PbPbusFsm4NoDataErr.Size()

	if fldName == "PbPbusFsm5PktTooSmallErr" {
		return offset
	}
	offset += mtr.PbPbusFsm5PktTooSmallErr.Size()

	if fldName == "LifIndTableRspErr" {
		return offset
	}
	offset += mtr.LifIndTableRspErr.Size()

	if fldName == "FlitCntOflowErr" {
		return offset
	}
	offset += mtr.FlitCntOflowErr.Size()

	if fldName == "LbPhvSopErr" {
		return offset
	}
	offset += mtr.LbPhvSopErr.Size()

	if fldName == "DcPhvSopErr" {
		return offset
	}
	offset += mtr.DcPhvSopErr.Size()

	if fldName == "LbPktSopErr" {
		return offset
	}
	offset += mtr.LbPktSopErr.Size()

	if fldName == "DcPktSopErr" {
		return offset
	}
	offset += mtr.DcPktSopErr.Size()

	if fldName == "SwPhvJabberErr" {
		return offset
	}
	offset += mtr.SwPhvJabberErr.Size()

	return offset
}

// SetPbPbusSopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPbPbusSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusSopErr"))
	return nil
}

// SetPbPbusEopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPbPbusEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusEopErr"))
	return nil
}

// SetMaSopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetMaSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaSopErr"))
	return nil
}

// SetMaEopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetMaEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaEopErr"))
	return nil
}

// SetPrPbusSopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPrPbusSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrPbusSopErr"))
	return nil
}

// SetPrPbusEopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPrPbusEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrPbusEopErr"))
	return nil
}

// SetPrResubPbusSopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPrResubPbusSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrResubPbusSopErr"))
	return nil
}

// SetPrResubPbusEopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPrResubPbusEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrResubPbusEopErr"))
	return nil
}

// SetPrResubSopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPrResubSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrResubSopErr"))
	return nil
}

// SetPrResubEopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPrResubEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrResubEopErr"))
	return nil
}

// SetPbPbusFsm0NoDataErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPbPbusFsm0NoDataErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusFsm0NoDataErr"))
	return nil
}

// SetPbPbusFsm1NoDataErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPbPbusFsm1NoDataErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusFsm1NoDataErr"))
	return nil
}

// SetPbPbusFsm2NoDataErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPbPbusFsm2NoDataErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusFsm2NoDataErr"))
	return nil
}

// SetPbPbusFsm3NoDataErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPbPbusFsm3NoDataErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusFsm3NoDataErr"))
	return nil
}

// SetPbPbusFsm4NoDataErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPbPbusFsm4NoDataErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusFsm4NoDataErr"))
	return nil
}

// SetPbPbusFsm5PktTooSmallErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetPbPbusFsm5PktTooSmallErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusFsm5PktTooSmallErr"))
	return nil
}

// SetLifIndTableRspErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetLifIndTableRspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LifIndTableRspErr"))
	return nil
}

// SetFlitCntOflowErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetFlitCntOflowErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FlitCntOflowErr"))
	return nil
}

// SetLbPhvSopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetLbPhvSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LbPhvSopErr"))
	return nil
}

// SetDcPhvSopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetDcPhvSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DcPhvSopErr"))
	return nil
}

// SetLbPktSopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetLbPktSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LbPktSopErr"))
	return nil
}

// SetDcPktSopErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetDcPktSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DcPktSopErr"))
	return nil
}

// SetSwPhvJabberErr sets cunter in shared memory
func (mtr *PrprpspintfatalMetrics) SetSwPhvJabberErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SwPhvJabberErr"))
	return nil
}

// PrprpspintfatalMetricsIterator is the iterator object
type PrprpspintfatalMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PrprpspintfatalMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PrprpspintfatalMetricsIterator) Next() *PrprpspintfatalMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PrprpspintfatalMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PrprpspintfatalMetricsIterator) Find(key uint64) (*PrprpspintfatalMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PrprpspintfatalMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PrprpspintfatalMetricsIterator) Create(key uint64) (*PrprpspintfatalMetrics, error) {
	tmtr := &PrprpspintfatalMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PrprpspintfatalMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PrprpspintfatalMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PrprpspintfatalMetricsIterator) Free() {
	it.iter.Free()
}

// NewPrprpspintfatalMetricsIterator returns an iterator
func NewPrprpspintfatalMetricsIterator() (*PrprpspintfatalMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PrprpspintfatalMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PrprpspintfatalMetricsIterator{iter: iter}, nil
}

type PrprpspintlifqstatemapMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	EccUncorrectable metrics.Counter

	EccCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PrprpspintlifqstatemapMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PrprpspintlifqstatemapMetrics) Size() int {
	sz := 0

	sz += mtr.EccUncorrectable.Size()

	sz += mtr.EccCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PrprpspintlifqstatemapMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.EccUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccUncorrectable.Size()

	mtr.EccCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PrprpspintlifqstatemapMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "EccUncorrectable" {
		return offset
	}
	offset += mtr.EccUncorrectable.Size()

	if fldName == "EccCorrectable" {
		return offset
	}
	offset += mtr.EccCorrectable.Size()

	return offset
}

// SetEccUncorrectable sets cunter in shared memory
func (mtr *PrprpspintlifqstatemapMetrics) SetEccUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccUncorrectable"))
	return nil
}

// SetEccCorrectable sets cunter in shared memory
func (mtr *PrprpspintlifqstatemapMetrics) SetEccCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccCorrectable"))
	return nil
}

// PrprpspintlifqstatemapMetricsIterator is the iterator object
type PrprpspintlifqstatemapMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PrprpspintlifqstatemapMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PrprpspintlifqstatemapMetricsIterator) Next() *PrprpspintlifqstatemapMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PrprpspintlifqstatemapMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PrprpspintlifqstatemapMetricsIterator) Find(key uint64) (*PrprpspintlifqstatemapMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PrprpspintlifqstatemapMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PrprpspintlifqstatemapMetricsIterator) Create(key uint64) (*PrprpspintlifqstatemapMetrics, error) {
	tmtr := &PrprpspintlifqstatemapMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PrprpspintlifqstatemapMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PrprpspintlifqstatemapMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PrprpspintlifqstatemapMetricsIterator) Free() {
	it.iter.Free()
}

// NewPrprpspintlifqstatemapMetricsIterator returns an iterator
func NewPrprpspintlifqstatemapMetricsIterator() (*PrprpspintlifqstatemapMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PrprpspintlifqstatemapMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PrprpspintlifqstatemapMetricsIterator{iter: iter}, nil
}

type PrprpspintswphvmemMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	EccUncorrectable metrics.Counter

	EccCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PrprpspintswphvmemMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PrprpspintswphvmemMetrics) Size() int {
	sz := 0

	sz += mtr.EccUncorrectable.Size()

	sz += mtr.EccCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PrprpspintswphvmemMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.EccUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccUncorrectable.Size()

	mtr.EccCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PrprpspintswphvmemMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "EccUncorrectable" {
		return offset
	}
	offset += mtr.EccUncorrectable.Size()

	if fldName == "EccCorrectable" {
		return offset
	}
	offset += mtr.EccCorrectable.Size()

	return offset
}

// SetEccUncorrectable sets cunter in shared memory
func (mtr *PrprpspintswphvmemMetrics) SetEccUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccUncorrectable"))
	return nil
}

// SetEccCorrectable sets cunter in shared memory
func (mtr *PrprpspintswphvmemMetrics) SetEccCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccCorrectable"))
	return nil
}

// PrprpspintswphvmemMetricsIterator is the iterator object
type PrprpspintswphvmemMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PrprpspintswphvmemMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PrprpspintswphvmemMetricsIterator) Next() *PrprpspintswphvmemMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PrprpspintswphvmemMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PrprpspintswphvmemMetricsIterator) Find(key uint64) (*PrprpspintswphvmemMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PrprpspintswphvmemMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PrprpspintswphvmemMetricsIterator) Create(key uint64) (*PrprpspintswphvmemMetrics, error) {
	tmtr := &PrprpspintswphvmemMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PrprpspintswphvmemMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PrprpspintswphvmemMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PrprpspintswphvmemMetricsIterator) Free() {
	it.iter.Free()
}

// NewPrprpspintswphvmemMetricsIterator returns an iterator
func NewPrprpspintswphvmemMetricsIterator() (*PrprpspintswphvmemMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PrprpspintswphvmemMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PrprpspintswphvmemMetricsIterator{iter: iter}, nil
}

type PrprprdinteccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RdataMemUncorrectable metrics.Counter

	RdataMemCorrectable metrics.Counter

	PktMemUncorrectable metrics.Counter

	PktMemCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PrprprdinteccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PrprprdinteccMetrics) Size() int {
	sz := 0

	sz += mtr.RdataMemUncorrectable.Size()

	sz += mtr.RdataMemCorrectable.Size()

	sz += mtr.PktMemUncorrectable.Size()

	sz += mtr.PktMemCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PrprprdinteccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RdataMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RdataMemUncorrectable.Size()

	mtr.RdataMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RdataMemCorrectable.Size()

	mtr.PktMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PktMemUncorrectable.Size()

	mtr.PktMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PktMemCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PrprprdinteccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RdataMemUncorrectable" {
		return offset
	}
	offset += mtr.RdataMemUncorrectable.Size()

	if fldName == "RdataMemCorrectable" {
		return offset
	}
	offset += mtr.RdataMemCorrectable.Size()

	if fldName == "PktMemUncorrectable" {
		return offset
	}
	offset += mtr.PktMemUncorrectable.Size()

	if fldName == "PktMemCorrectable" {
		return offset
	}
	offset += mtr.PktMemCorrectable.Size()

	return offset
}

// SetRdataMemUncorrectable sets cunter in shared memory
func (mtr *PrprprdinteccMetrics) SetRdataMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdataMemUncorrectable"))
	return nil
}

// SetRdataMemCorrectable sets cunter in shared memory
func (mtr *PrprprdinteccMetrics) SetRdataMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdataMemCorrectable"))
	return nil
}

// SetPktMemUncorrectable sets cunter in shared memory
func (mtr *PrprprdinteccMetrics) SetPktMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktMemUncorrectable"))
	return nil
}

// SetPktMemCorrectable sets cunter in shared memory
func (mtr *PrprprdinteccMetrics) SetPktMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktMemCorrectable"))
	return nil
}

// PrprprdinteccMetricsIterator is the iterator object
type PrprprdinteccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PrprprdinteccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PrprprdinteccMetricsIterator) Next() *PrprprdinteccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PrprprdinteccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PrprprdinteccMetricsIterator) Find(key uint64) (*PrprprdinteccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PrprprdinteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PrprprdinteccMetricsIterator) Create(key uint64) (*PrprprdinteccMetrics, error) {
	tmtr := &PrprprdinteccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PrprprdinteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PrprprdinteccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PrprprdinteccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPrprprdinteccMetricsIterator returns an iterator
func NewPrprprdinteccMetricsIterator() (*PrprprdinteccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PrprprdinteccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PrprprdinteccMetricsIterator{iter: iter}, nil
}

type PrprprdintfifoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	WrLatFfOvflow metrics.Counter

	RdLatFfOvflow metrics.Counter

	WdataFfOvflow metrics.Counter

	RcvStgFfOvflow metrics.Counter

	CmdflitFfOvflow metrics.Counter

	CmdFfOvflow metrics.Counter

	PktFfOvflow metrics.Counter

	PktOrderFfOvflow metrics.Counter

	PktStgFfOvflow metrics.Counter

	WrMemFfOvflow metrics.Counter

	DfenceFfOvflow metrics.Counter

	FfenceFfOvflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PrprprdintfifoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PrprprdintfifoMetrics) Size() int {
	sz := 0

	sz += mtr.WrLatFfOvflow.Size()

	sz += mtr.RdLatFfOvflow.Size()

	sz += mtr.WdataFfOvflow.Size()

	sz += mtr.RcvStgFfOvflow.Size()

	sz += mtr.CmdflitFfOvflow.Size()

	sz += mtr.CmdFfOvflow.Size()

	sz += mtr.PktFfOvflow.Size()

	sz += mtr.PktOrderFfOvflow.Size()

	sz += mtr.PktStgFfOvflow.Size()

	sz += mtr.WrMemFfOvflow.Size()

	sz += mtr.DfenceFfOvflow.Size()

	sz += mtr.FfenceFfOvflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PrprprdintfifoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.WrLatFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.WrLatFfOvflow.Size()

	mtr.RdLatFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.RdLatFfOvflow.Size()

	mtr.WdataFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.WdataFfOvflow.Size()

	mtr.RcvStgFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvStgFfOvflow.Size()

	mtr.CmdflitFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CmdflitFfOvflow.Size()

	mtr.CmdFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CmdFfOvflow.Size()

	mtr.PktFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktFfOvflow.Size()

	mtr.PktOrderFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktOrderFfOvflow.Size()

	mtr.PktStgFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktStgFfOvflow.Size()

	mtr.WrMemFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.WrMemFfOvflow.Size()

	mtr.DfenceFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DfenceFfOvflow.Size()

	mtr.FfenceFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.FfenceFfOvflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PrprprdintfifoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "WrLatFfOvflow" {
		return offset
	}
	offset += mtr.WrLatFfOvflow.Size()

	if fldName == "RdLatFfOvflow" {
		return offset
	}
	offset += mtr.RdLatFfOvflow.Size()

	if fldName == "WdataFfOvflow" {
		return offset
	}
	offset += mtr.WdataFfOvflow.Size()

	if fldName == "RcvStgFfOvflow" {
		return offset
	}
	offset += mtr.RcvStgFfOvflow.Size()

	if fldName == "CmdflitFfOvflow" {
		return offset
	}
	offset += mtr.CmdflitFfOvflow.Size()

	if fldName == "CmdFfOvflow" {
		return offset
	}
	offset += mtr.CmdFfOvflow.Size()

	if fldName == "PktFfOvflow" {
		return offset
	}
	offset += mtr.PktFfOvflow.Size()

	if fldName == "PktOrderFfOvflow" {
		return offset
	}
	offset += mtr.PktOrderFfOvflow.Size()

	if fldName == "PktStgFfOvflow" {
		return offset
	}
	offset += mtr.PktStgFfOvflow.Size()

	if fldName == "WrMemFfOvflow" {
		return offset
	}
	offset += mtr.WrMemFfOvflow.Size()

	if fldName == "DfenceFfOvflow" {
		return offset
	}
	offset += mtr.DfenceFfOvflow.Size()

	if fldName == "FfenceFfOvflow" {
		return offset
	}
	offset += mtr.FfenceFfOvflow.Size()

	return offset
}

// SetWrLatFfOvflow sets cunter in shared memory
func (mtr *PrprprdintfifoMetrics) SetWrLatFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrLatFfOvflow"))
	return nil
}

// SetRdLatFfOvflow sets cunter in shared memory
func (mtr *PrprprdintfifoMetrics) SetRdLatFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdLatFfOvflow"))
	return nil
}

// SetWdataFfOvflow sets cunter in shared memory
func (mtr *PrprprdintfifoMetrics) SetWdataFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WdataFfOvflow"))
	return nil
}

// SetRcvStgFfOvflow sets cunter in shared memory
func (mtr *PrprprdintfifoMetrics) SetRcvStgFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvStgFfOvflow"))
	return nil
}

// SetCmdflitFfOvflow sets cunter in shared memory
func (mtr *PrprprdintfifoMetrics) SetCmdflitFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CmdflitFfOvflow"))
	return nil
}

// SetCmdFfOvflow sets cunter in shared memory
func (mtr *PrprprdintfifoMetrics) SetCmdFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CmdFfOvflow"))
	return nil
}

// SetPktFfOvflow sets cunter in shared memory
func (mtr *PrprprdintfifoMetrics) SetPktFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktFfOvflow"))
	return nil
}

// SetPktOrderFfOvflow sets cunter in shared memory
func (mtr *PrprprdintfifoMetrics) SetPktOrderFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktOrderFfOvflow"))
	return nil
}

// SetPktStgFfOvflow sets cunter in shared memory
func (mtr *PrprprdintfifoMetrics) SetPktStgFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktStgFfOvflow"))
	return nil
}

// SetWrMemFfOvflow sets cunter in shared memory
func (mtr *PrprprdintfifoMetrics) SetWrMemFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrMemFfOvflow"))
	return nil
}

// SetDfenceFfOvflow sets cunter in shared memory
func (mtr *PrprprdintfifoMetrics) SetDfenceFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DfenceFfOvflow"))
	return nil
}

// SetFfenceFfOvflow sets cunter in shared memory
func (mtr *PrprprdintfifoMetrics) SetFfenceFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FfenceFfOvflow"))
	return nil
}

// PrprprdintfifoMetricsIterator is the iterator object
type PrprprdintfifoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PrprprdintfifoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PrprprdintfifoMetricsIterator) Next() *PrprprdintfifoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PrprprdintfifoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PrprprdintfifoMetricsIterator) Find(key uint64) (*PrprprdintfifoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PrprprdintfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PrprprdintfifoMetricsIterator) Create(key uint64) (*PrprprdintfifoMetrics, error) {
	tmtr := &PrprprdintfifoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PrprprdintfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PrprprdintfifoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PrprprdintfifoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPrprprdintfifoMetricsIterator returns an iterator
func NewPrprprdintfifoMetricsIterator() (*PrprprdintfifoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PrprprdintfifoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PrprprdintfifoMetricsIterator{iter: iter}, nil
}

type Prprprdintgrp1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RcvPhvDmaPtr metrics.Counter

	RcvPhvAddr metrics.Counter

	RcvExceed_16Byte metrics.Counter

	RcvPhvNotSop metrics.Counter

	RcvPktOrderFfFull metrics.Counter

	RcvPendPhvMoreThan_2 metrics.Counter

	RcvPendPhvLess metrics.Counter

	RcvCmdNopEop metrics.Counter

	RcvCmdOutNotSop metrics.Counter

	RcvNoDataButPkt2MemCmd metrics.Counter

	RcvNoDataButSkipCmd metrics.Counter

	RcvMem2PktSeen metrics.Counter

	RcvPhv2PktSeen metrics.Counter

	RcvPhvEopNoCmdEop metrics.Counter

	RcvM2MDstNotSeen metrics.Counter

	RcvM2MSrcNotSeen metrics.Counter

	RdreqInvalidCmdSeen metrics.Counter

	RdreqMem2MemPsizeZero metrics.Counter

	RdreqM2MPhv2MemExceed_16Byte metrics.Counter

	RdreqPhv2MemFenceExceed_16Byte metrics.Counter

	RdreqPkt2MemPsizeZero metrics.Counter

	RdreqSkipPsizeZero metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Prprprdintgrp1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Prprprdintgrp1Metrics) Size() int {
	sz := 0

	sz += mtr.RcvPhvDmaPtr.Size()

	sz += mtr.RcvPhvAddr.Size()

	sz += mtr.RcvExceed_16Byte.Size()

	sz += mtr.RcvPhvNotSop.Size()

	sz += mtr.RcvPktOrderFfFull.Size()

	sz += mtr.RcvPendPhvMoreThan_2.Size()

	sz += mtr.RcvPendPhvLess.Size()

	sz += mtr.RcvCmdNopEop.Size()

	sz += mtr.RcvCmdOutNotSop.Size()

	sz += mtr.RcvNoDataButPkt2MemCmd.Size()

	sz += mtr.RcvNoDataButSkipCmd.Size()

	sz += mtr.RcvMem2PktSeen.Size()

	sz += mtr.RcvPhv2PktSeen.Size()

	sz += mtr.RcvPhvEopNoCmdEop.Size()

	sz += mtr.RcvM2MDstNotSeen.Size()

	sz += mtr.RcvM2MSrcNotSeen.Size()

	sz += mtr.RdreqInvalidCmdSeen.Size()

	sz += mtr.RdreqMem2MemPsizeZero.Size()

	sz += mtr.RdreqM2MPhv2MemExceed_16Byte.Size()

	sz += mtr.RdreqPhv2MemFenceExceed_16Byte.Size()

	sz += mtr.RdreqPkt2MemPsizeZero.Size()

	sz += mtr.RdreqSkipPsizeZero.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Prprprdintgrp1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RcvPhvDmaPtr = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPhvDmaPtr.Size()

	mtr.RcvPhvAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPhvAddr.Size()

	mtr.RcvExceed_16Byte = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvExceed_16Byte.Size()

	mtr.RcvPhvNotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPhvNotSop.Size()

	mtr.RcvPktOrderFfFull = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPktOrderFfFull.Size()

	mtr.RcvPendPhvMoreThan_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPendPhvMoreThan_2.Size()

	mtr.RcvPendPhvLess = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPendPhvLess.Size()

	mtr.RcvCmdNopEop = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvCmdNopEop.Size()

	mtr.RcvCmdOutNotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvCmdOutNotSop.Size()

	mtr.RcvNoDataButPkt2MemCmd = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvNoDataButPkt2MemCmd.Size()

	mtr.RcvNoDataButSkipCmd = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvNoDataButSkipCmd.Size()

	mtr.RcvMem2PktSeen = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvMem2PktSeen.Size()

	mtr.RcvPhv2PktSeen = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPhv2PktSeen.Size()

	mtr.RcvPhvEopNoCmdEop = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPhvEopNoCmdEop.Size()

	mtr.RcvM2MDstNotSeen = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvM2MDstNotSeen.Size()

	mtr.RcvM2MSrcNotSeen = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvM2MSrcNotSeen.Size()

	mtr.RdreqInvalidCmdSeen = mtr.metrics.GetCounter(offset)
	offset += mtr.RdreqInvalidCmdSeen.Size()

	mtr.RdreqMem2MemPsizeZero = mtr.metrics.GetCounter(offset)
	offset += mtr.RdreqMem2MemPsizeZero.Size()

	mtr.RdreqM2MPhv2MemExceed_16Byte = mtr.metrics.GetCounter(offset)
	offset += mtr.RdreqM2MPhv2MemExceed_16Byte.Size()

	mtr.RdreqPhv2MemFenceExceed_16Byte = mtr.metrics.GetCounter(offset)
	offset += mtr.RdreqPhv2MemFenceExceed_16Byte.Size()

	mtr.RdreqPkt2MemPsizeZero = mtr.metrics.GetCounter(offset)
	offset += mtr.RdreqPkt2MemPsizeZero.Size()

	mtr.RdreqSkipPsizeZero = mtr.metrics.GetCounter(offset)
	offset += mtr.RdreqSkipPsizeZero.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Prprprdintgrp1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RcvPhvDmaPtr" {
		return offset
	}
	offset += mtr.RcvPhvDmaPtr.Size()

	if fldName == "RcvPhvAddr" {
		return offset
	}
	offset += mtr.RcvPhvAddr.Size()

	if fldName == "RcvExceed_16Byte" {
		return offset
	}
	offset += mtr.RcvExceed_16Byte.Size()

	if fldName == "RcvPhvNotSop" {
		return offset
	}
	offset += mtr.RcvPhvNotSop.Size()

	if fldName == "RcvPktOrderFfFull" {
		return offset
	}
	offset += mtr.RcvPktOrderFfFull.Size()

	if fldName == "RcvPendPhvMoreThan_2" {
		return offset
	}
	offset += mtr.RcvPendPhvMoreThan_2.Size()

	if fldName == "RcvPendPhvLess" {
		return offset
	}
	offset += mtr.RcvPendPhvLess.Size()

	if fldName == "RcvCmdNopEop" {
		return offset
	}
	offset += mtr.RcvCmdNopEop.Size()

	if fldName == "RcvCmdOutNotSop" {
		return offset
	}
	offset += mtr.RcvCmdOutNotSop.Size()

	if fldName == "RcvNoDataButPkt2MemCmd" {
		return offset
	}
	offset += mtr.RcvNoDataButPkt2MemCmd.Size()

	if fldName == "RcvNoDataButSkipCmd" {
		return offset
	}
	offset += mtr.RcvNoDataButSkipCmd.Size()

	if fldName == "RcvMem2PktSeen" {
		return offset
	}
	offset += mtr.RcvMem2PktSeen.Size()

	if fldName == "RcvPhv2PktSeen" {
		return offset
	}
	offset += mtr.RcvPhv2PktSeen.Size()

	if fldName == "RcvPhvEopNoCmdEop" {
		return offset
	}
	offset += mtr.RcvPhvEopNoCmdEop.Size()

	if fldName == "RcvM2MDstNotSeen" {
		return offset
	}
	offset += mtr.RcvM2MDstNotSeen.Size()

	if fldName == "RcvM2MSrcNotSeen" {
		return offset
	}
	offset += mtr.RcvM2MSrcNotSeen.Size()

	if fldName == "RdreqInvalidCmdSeen" {
		return offset
	}
	offset += mtr.RdreqInvalidCmdSeen.Size()

	if fldName == "RdreqMem2MemPsizeZero" {
		return offset
	}
	offset += mtr.RdreqMem2MemPsizeZero.Size()

	if fldName == "RdreqM2MPhv2MemExceed_16Byte" {
		return offset
	}
	offset += mtr.RdreqM2MPhv2MemExceed_16Byte.Size()

	if fldName == "RdreqPhv2MemFenceExceed_16Byte" {
		return offset
	}
	offset += mtr.RdreqPhv2MemFenceExceed_16Byte.Size()

	if fldName == "RdreqPkt2MemPsizeZero" {
		return offset
	}
	offset += mtr.RdreqPkt2MemPsizeZero.Size()

	if fldName == "RdreqSkipPsizeZero" {
		return offset
	}
	offset += mtr.RdreqSkipPsizeZero.Size()

	return offset
}

// SetRcvPhvDmaPtr sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvPhvDmaPtr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPhvDmaPtr"))
	return nil
}

// SetRcvPhvAddr sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvPhvAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPhvAddr"))
	return nil
}

// SetRcvExceed_16Byte sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvExceed_16Byte(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvExceed_16Byte"))
	return nil
}

// SetRcvPhvNotSop sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvPhvNotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPhvNotSop"))
	return nil
}

// SetRcvPktOrderFfFull sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvPktOrderFfFull(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPktOrderFfFull"))
	return nil
}

// SetRcvPendPhvMoreThan_2 sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvPendPhvMoreThan_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPendPhvMoreThan_2"))
	return nil
}

// SetRcvPendPhvLess sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvPendPhvLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPendPhvLess"))
	return nil
}

// SetRcvCmdNopEop sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvCmdNopEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvCmdNopEop"))
	return nil
}

// SetRcvCmdOutNotSop sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvCmdOutNotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvCmdOutNotSop"))
	return nil
}

// SetRcvNoDataButPkt2MemCmd sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvNoDataButPkt2MemCmd(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvNoDataButPkt2MemCmd"))
	return nil
}

// SetRcvNoDataButSkipCmd sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvNoDataButSkipCmd(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvNoDataButSkipCmd"))
	return nil
}

// SetRcvMem2PktSeen sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvMem2PktSeen(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvMem2PktSeen"))
	return nil
}

// SetRcvPhv2PktSeen sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvPhv2PktSeen(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPhv2PktSeen"))
	return nil
}

// SetRcvPhvEopNoCmdEop sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvPhvEopNoCmdEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPhvEopNoCmdEop"))
	return nil
}

// SetRcvM2MDstNotSeen sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvM2MDstNotSeen(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvM2MDstNotSeen"))
	return nil
}

// SetRcvM2MSrcNotSeen sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRcvM2MSrcNotSeen(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvM2MSrcNotSeen"))
	return nil
}

// SetRdreqInvalidCmdSeen sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRdreqInvalidCmdSeen(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdreqInvalidCmdSeen"))
	return nil
}

// SetRdreqMem2MemPsizeZero sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRdreqMem2MemPsizeZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdreqMem2MemPsizeZero"))
	return nil
}

// SetRdreqM2MPhv2MemExceed_16Byte sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRdreqM2MPhv2MemExceed_16Byte(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdreqM2MPhv2MemExceed_16Byte"))
	return nil
}

// SetRdreqPhv2MemFenceExceed_16Byte sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRdreqPhv2MemFenceExceed_16Byte(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdreqPhv2MemFenceExceed_16Byte"))
	return nil
}

// SetRdreqPkt2MemPsizeZero sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRdreqPkt2MemPsizeZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdreqPkt2MemPsizeZero"))
	return nil
}

// SetRdreqSkipPsizeZero sets cunter in shared memory
func (mtr *Prprprdintgrp1Metrics) SetRdreqSkipPsizeZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdreqSkipPsizeZero"))
	return nil
}

// Prprprdintgrp1MetricsIterator is the iterator object
type Prprprdintgrp1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Prprprdintgrp1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Prprprdintgrp1MetricsIterator) Next() *Prprprdintgrp1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Prprprdintgrp1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Prprprdintgrp1MetricsIterator) Find(key uint64) (*Prprprdintgrp1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Prprprdintgrp1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Prprprdintgrp1MetricsIterator) Create(key uint64) (*Prprprdintgrp1Metrics, error) {
	tmtr := &Prprprdintgrp1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Prprprdintgrp1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Prprprdintgrp1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Prprprdintgrp1MetricsIterator) Free() {
	it.iter.Free()
}

// NewPrprprdintgrp1MetricsIterator returns an iterator
func NewPrprprdintgrp1MetricsIterator() (*Prprprdintgrp1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Prprprdintgrp1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Prprprdintgrp1MetricsIterator{iter: iter}, nil
}

type Prprprdintgrp2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	WrAxiRdErrRecovery metrics.Counter

	WrAxiRdResp metrics.Counter

	WrNotEnufPktBytes metrics.Counter

	WrInvalidCmd metrics.Counter

	WrSeqId metrics.Counter

	WrNoDataPktCmd metrics.Counter

	RdrspAxi metrics.Counter

	WrrspAxi metrics.Counter

	PktNotSop metrics.Counter

	SpuriousRdResp metrics.Counter

	RdrspAxiIdOutOfRange metrics.Counter

	SpuriousWrResp metrics.Counter

	WrreqInfoFirstMissing metrics.Counter

	WrreqNumBytesZero metrics.Counter

	WrreqNumBytesMoreThan_64 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Prprprdintgrp2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Prprprdintgrp2Metrics) Size() int {
	sz := 0

	sz += mtr.WrAxiRdErrRecovery.Size()

	sz += mtr.WrAxiRdResp.Size()

	sz += mtr.WrNotEnufPktBytes.Size()

	sz += mtr.WrInvalidCmd.Size()

	sz += mtr.WrSeqId.Size()

	sz += mtr.WrNoDataPktCmd.Size()

	sz += mtr.RdrspAxi.Size()

	sz += mtr.WrrspAxi.Size()

	sz += mtr.PktNotSop.Size()

	sz += mtr.SpuriousRdResp.Size()

	sz += mtr.RdrspAxiIdOutOfRange.Size()

	sz += mtr.SpuriousWrResp.Size()

	sz += mtr.WrreqInfoFirstMissing.Size()

	sz += mtr.WrreqNumBytesZero.Size()

	sz += mtr.WrreqNumBytesMoreThan_64.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Prprprdintgrp2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.WrAxiRdErrRecovery = mtr.metrics.GetCounter(offset)
	offset += mtr.WrAxiRdErrRecovery.Size()

	mtr.WrAxiRdResp = mtr.metrics.GetCounter(offset)
	offset += mtr.WrAxiRdResp.Size()

	mtr.WrNotEnufPktBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.WrNotEnufPktBytes.Size()

	mtr.WrInvalidCmd = mtr.metrics.GetCounter(offset)
	offset += mtr.WrInvalidCmd.Size()

	mtr.WrSeqId = mtr.metrics.GetCounter(offset)
	offset += mtr.WrSeqId.Size()

	mtr.WrNoDataPktCmd = mtr.metrics.GetCounter(offset)
	offset += mtr.WrNoDataPktCmd.Size()

	mtr.RdrspAxi = mtr.metrics.GetCounter(offset)
	offset += mtr.RdrspAxi.Size()

	mtr.WrrspAxi = mtr.metrics.GetCounter(offset)
	offset += mtr.WrrspAxi.Size()

	mtr.PktNotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.PktNotSop.Size()

	mtr.SpuriousRdResp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousRdResp.Size()

	mtr.RdrspAxiIdOutOfRange = mtr.metrics.GetCounter(offset)
	offset += mtr.RdrspAxiIdOutOfRange.Size()

	mtr.SpuriousWrResp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousWrResp.Size()

	mtr.WrreqInfoFirstMissing = mtr.metrics.GetCounter(offset)
	offset += mtr.WrreqInfoFirstMissing.Size()

	mtr.WrreqNumBytesZero = mtr.metrics.GetCounter(offset)
	offset += mtr.WrreqNumBytesZero.Size()

	mtr.WrreqNumBytesMoreThan_64 = mtr.metrics.GetCounter(offset)
	offset += mtr.WrreqNumBytesMoreThan_64.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Prprprdintgrp2Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "WrAxiRdErrRecovery" {
		return offset
	}
	offset += mtr.WrAxiRdErrRecovery.Size()

	if fldName == "WrAxiRdResp" {
		return offset
	}
	offset += mtr.WrAxiRdResp.Size()

	if fldName == "WrNotEnufPktBytes" {
		return offset
	}
	offset += mtr.WrNotEnufPktBytes.Size()

	if fldName == "WrInvalidCmd" {
		return offset
	}
	offset += mtr.WrInvalidCmd.Size()

	if fldName == "WrSeqId" {
		return offset
	}
	offset += mtr.WrSeqId.Size()

	if fldName == "WrNoDataPktCmd" {
		return offset
	}
	offset += mtr.WrNoDataPktCmd.Size()

	if fldName == "RdrspAxi" {
		return offset
	}
	offset += mtr.RdrspAxi.Size()

	if fldName == "WrrspAxi" {
		return offset
	}
	offset += mtr.WrrspAxi.Size()

	if fldName == "PktNotSop" {
		return offset
	}
	offset += mtr.PktNotSop.Size()

	if fldName == "SpuriousRdResp" {
		return offset
	}
	offset += mtr.SpuriousRdResp.Size()

	if fldName == "RdrspAxiIdOutOfRange" {
		return offset
	}
	offset += mtr.RdrspAxiIdOutOfRange.Size()

	if fldName == "SpuriousWrResp" {
		return offset
	}
	offset += mtr.SpuriousWrResp.Size()

	if fldName == "WrreqInfoFirstMissing" {
		return offset
	}
	offset += mtr.WrreqInfoFirstMissing.Size()

	if fldName == "WrreqNumBytesZero" {
		return offset
	}
	offset += mtr.WrreqNumBytesZero.Size()

	if fldName == "WrreqNumBytesMoreThan_64" {
		return offset
	}
	offset += mtr.WrreqNumBytesMoreThan_64.Size()

	return offset
}

// SetWrAxiRdErrRecovery sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetWrAxiRdErrRecovery(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrAxiRdErrRecovery"))
	return nil
}

// SetWrAxiRdResp sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetWrAxiRdResp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrAxiRdResp"))
	return nil
}

// SetWrNotEnufPktBytes sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetWrNotEnufPktBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrNotEnufPktBytes"))
	return nil
}

// SetWrInvalidCmd sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetWrInvalidCmd(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrInvalidCmd"))
	return nil
}

// SetWrSeqId sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetWrSeqId(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrSeqId"))
	return nil
}

// SetWrNoDataPktCmd sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetWrNoDataPktCmd(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrNoDataPktCmd"))
	return nil
}

// SetRdrspAxi sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetRdrspAxi(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdrspAxi"))
	return nil
}

// SetWrrspAxi sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetWrrspAxi(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrrspAxi"))
	return nil
}

// SetPktNotSop sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetPktNotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktNotSop"))
	return nil
}

// SetSpuriousRdResp sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetSpuriousRdResp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousRdResp"))
	return nil
}

// SetRdrspAxiIdOutOfRange sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetRdrspAxiIdOutOfRange(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdrspAxiIdOutOfRange"))
	return nil
}

// SetSpuriousWrResp sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetSpuriousWrResp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousWrResp"))
	return nil
}

// SetWrreqInfoFirstMissing sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetWrreqInfoFirstMissing(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrreqInfoFirstMissing"))
	return nil
}

// SetWrreqNumBytesZero sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetWrreqNumBytesZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrreqNumBytesZero"))
	return nil
}

// SetWrreqNumBytesMoreThan_64 sets cunter in shared memory
func (mtr *Prprprdintgrp2Metrics) SetWrreqNumBytesMoreThan_64(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrreqNumBytesMoreThan_64"))
	return nil
}

// Prprprdintgrp2MetricsIterator is the iterator object
type Prprprdintgrp2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Prprprdintgrp2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Prprprdintgrp2MetricsIterator) Next() *Prprprdintgrp2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Prprprdintgrp2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Prprprdintgrp2MetricsIterator) Find(key uint64) (*Prprprdintgrp2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Prprprdintgrp2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Prprprdintgrp2MetricsIterator) Create(key uint64) (*Prprprdintgrp2Metrics, error) {
	tmtr := &Prprprdintgrp2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Prprprdintgrp2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Prprprdintgrp2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Prprprdintgrp2MetricsIterator) Free() {
	it.iter.Free()
}

// NewPrprprdintgrp2MetricsIterator returns an iterator
func NewPrprprdintgrp2MetricsIterator() (*Prprprdintgrp2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Prprprdintgrp2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Prprprdintgrp2MetricsIterator{iter: iter}, nil
}

type PrprprdintintfMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PsPktErr metrics.Counter

	PsPktSopErr metrics.Counter

	PsPktEopErr metrics.Counter

	PsResubPktErr metrics.Counter

	PsResubPktSopErr metrics.Counter

	PsResubPktEopErr metrics.Counter

	PsResubPhvErr metrics.Counter

	PsResubPhvSopErr metrics.Counter

	PsResubPhvEopErr metrics.Counter

	MaErr metrics.Counter

	MaSopErr metrics.Counter

	MaEopErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PrprprdintintfMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PrprprdintintfMetrics) Size() int {
	sz := 0

	sz += mtr.PsPktErr.Size()

	sz += mtr.PsPktSopErr.Size()

	sz += mtr.PsPktEopErr.Size()

	sz += mtr.PsResubPktErr.Size()

	sz += mtr.PsResubPktSopErr.Size()

	sz += mtr.PsResubPktEopErr.Size()

	sz += mtr.PsResubPhvErr.Size()

	sz += mtr.PsResubPhvSopErr.Size()

	sz += mtr.PsResubPhvEopErr.Size()

	sz += mtr.MaErr.Size()

	sz += mtr.MaSopErr.Size()

	sz += mtr.MaEopErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PrprprdintintfMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PsPktErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PsPktErr.Size()

	mtr.PsPktSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PsPktSopErr.Size()

	mtr.PsPktEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PsPktEopErr.Size()

	mtr.PsResubPktErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PsResubPktErr.Size()

	mtr.PsResubPktSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PsResubPktSopErr.Size()

	mtr.PsResubPktEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PsResubPktEopErr.Size()

	mtr.PsResubPhvErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PsResubPhvErr.Size()

	mtr.PsResubPhvSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PsResubPhvSopErr.Size()

	mtr.PsResubPhvEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PsResubPhvEopErr.Size()

	mtr.MaErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaErr.Size()

	mtr.MaSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaSopErr.Size()

	mtr.MaEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaEopErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PrprprdintintfMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PsPktErr" {
		return offset
	}
	offset += mtr.PsPktErr.Size()

	if fldName == "PsPktSopErr" {
		return offset
	}
	offset += mtr.PsPktSopErr.Size()

	if fldName == "PsPktEopErr" {
		return offset
	}
	offset += mtr.PsPktEopErr.Size()

	if fldName == "PsResubPktErr" {
		return offset
	}
	offset += mtr.PsResubPktErr.Size()

	if fldName == "PsResubPktSopErr" {
		return offset
	}
	offset += mtr.PsResubPktSopErr.Size()

	if fldName == "PsResubPktEopErr" {
		return offset
	}
	offset += mtr.PsResubPktEopErr.Size()

	if fldName == "PsResubPhvErr" {
		return offset
	}
	offset += mtr.PsResubPhvErr.Size()

	if fldName == "PsResubPhvSopErr" {
		return offset
	}
	offset += mtr.PsResubPhvSopErr.Size()

	if fldName == "PsResubPhvEopErr" {
		return offset
	}
	offset += mtr.PsResubPhvEopErr.Size()

	if fldName == "MaErr" {
		return offset
	}
	offset += mtr.MaErr.Size()

	if fldName == "MaSopErr" {
		return offset
	}
	offset += mtr.MaSopErr.Size()

	if fldName == "MaEopErr" {
		return offset
	}
	offset += mtr.MaEopErr.Size()

	return offset
}

// SetPsPktErr sets cunter in shared memory
func (mtr *PrprprdintintfMetrics) SetPsPktErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PsPktErr"))
	return nil
}

// SetPsPktSopErr sets cunter in shared memory
func (mtr *PrprprdintintfMetrics) SetPsPktSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PsPktSopErr"))
	return nil
}

// SetPsPktEopErr sets cunter in shared memory
func (mtr *PrprprdintintfMetrics) SetPsPktEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PsPktEopErr"))
	return nil
}

// SetPsResubPktErr sets cunter in shared memory
func (mtr *PrprprdintintfMetrics) SetPsResubPktErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PsResubPktErr"))
	return nil
}

// SetPsResubPktSopErr sets cunter in shared memory
func (mtr *PrprprdintintfMetrics) SetPsResubPktSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PsResubPktSopErr"))
	return nil
}

// SetPsResubPktEopErr sets cunter in shared memory
func (mtr *PrprprdintintfMetrics) SetPsResubPktEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PsResubPktEopErr"))
	return nil
}

// SetPsResubPhvErr sets cunter in shared memory
func (mtr *PrprprdintintfMetrics) SetPsResubPhvErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PsResubPhvErr"))
	return nil
}

// SetPsResubPhvSopErr sets cunter in shared memory
func (mtr *PrprprdintintfMetrics) SetPsResubPhvSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PsResubPhvSopErr"))
	return nil
}

// SetPsResubPhvEopErr sets cunter in shared memory
func (mtr *PrprprdintintfMetrics) SetPsResubPhvEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PsResubPhvEopErr"))
	return nil
}

// SetMaErr sets cunter in shared memory
func (mtr *PrprprdintintfMetrics) SetMaErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaErr"))
	return nil
}

// SetMaSopErr sets cunter in shared memory
func (mtr *PrprprdintintfMetrics) SetMaSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaSopErr"))
	return nil
}

// SetMaEopErr sets cunter in shared memory
func (mtr *PrprprdintintfMetrics) SetMaEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaEopErr"))
	return nil
}

// PrprprdintintfMetricsIterator is the iterator object
type PrprprdintintfMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PrprprdintintfMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PrprprdintintfMetricsIterator) Next() *PrprprdintintfMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PrprprdintintfMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PrprprdintintfMetricsIterator) Find(key uint64) (*PrprprdintintfMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PrprprdintintfMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PrprprdintintfMetricsIterator) Create(key uint64) (*PrprprdintintfMetrics, error) {
	tmtr := &PrprprdintintfMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PrprprdintintfMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PrprprdintintfMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PrprprdintintfMetricsIterator) Free() {
	it.iter.Free()
}

// NewPrprprdintintfMetricsIterator returns an iterator
func NewPrprprdintintfMetricsIterator() (*PrprprdintintfMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PrprprdintintfMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PrprprdintintfMetricsIterator{iter: iter}, nil
}

type PpppintppMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PpsdSbe metrics.Counter

	PpsdDbe metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PpppintppMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PpppintppMetrics) Size() int {
	sz := 0

	sz += mtr.PpsdSbe.Size()

	sz += mtr.PpsdDbe.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PpppintppMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PpsdSbe = mtr.metrics.GetCounter(offset)
	offset += mtr.PpsdSbe.Size()

	mtr.PpsdDbe = mtr.metrics.GetCounter(offset)
	offset += mtr.PpsdDbe.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PpppintppMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PpsdSbe" {
		return offset
	}
	offset += mtr.PpsdSbe.Size()

	if fldName == "PpsdDbe" {
		return offset
	}
	offset += mtr.PpsdDbe.Size()

	return offset
}

// SetPpsdSbe sets cunter in shared memory
func (mtr *PpppintppMetrics) SetPpsdSbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PpsdSbe"))
	return nil
}

// SetPpsdDbe sets cunter in shared memory
func (mtr *PpppintppMetrics) SetPpsdDbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PpsdDbe"))
	return nil
}

// PpppintppMetricsIterator is the iterator object
type PpppintppMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PpppintppMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PpppintppMetricsIterator) Next() *PpppintppMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PpppintppMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PpppintppMetricsIterator) Find(key uint64) (*PpppintppMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PpppintppMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PpppintppMetricsIterator) Create(key uint64) (*PpppintppMetrics, error) {
	tmtr := &PpppintppMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PpppintppMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PpppintppMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PpppintppMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppintppMetricsIterator returns an iterator
func NewPpppintppMetricsIterator() (*PpppintppMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PpppintppMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PpppintppMetricsIterator{iter: iter}, nil
}

type Ppppportp0intpeccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RxbfrOverflow metrics.Counter

	ReplayBfrOverflow metrics.Counter

	Txbuf_0Uncorrectable metrics.Counter

	Txbuf_1Uncorrectable metrics.Counter

	Txbuf_2Uncorrectable metrics.Counter

	Txbuf_3Uncorrectable metrics.Counter

	Txbuf_0Correctable metrics.Counter

	Txbuf_1Correctable metrics.Counter

	Txbuf_2Correctable metrics.Counter

	Txbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportp0intpeccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportp0intpeccMetrics) Size() int {
	sz := 0

	sz += mtr.RxbfrOverflow.Size()

	sz += mtr.ReplayBfrOverflow.Size()

	sz += mtr.Txbuf_0Uncorrectable.Size()

	sz += mtr.Txbuf_1Uncorrectable.Size()

	sz += mtr.Txbuf_2Uncorrectable.Size()

	sz += mtr.Txbuf_3Uncorrectable.Size()

	sz += mtr.Txbuf_0Correctable.Size()

	sz += mtr.Txbuf_1Correctable.Size()

	sz += mtr.Txbuf_2Correctable.Size()

	sz += mtr.Txbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportp0intpeccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.RxbfrOverflow.Size()

	mtr.ReplayBfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ReplayBfrOverflow.Size()

	mtr.Txbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Uncorrectable.Size()

	mtr.Txbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Uncorrectable.Size()

	mtr.Txbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Uncorrectable.Size()

	mtr.Txbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Uncorrectable.Size()

	mtr.Txbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Correctable.Size()

	mtr.Txbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Correctable.Size()

	mtr.Txbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Correctable.Size()

	mtr.Txbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportp0intpeccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxbfrOverflow" {
		return offset
	}
	offset += mtr.RxbfrOverflow.Size()

	if fldName == "ReplayBfrOverflow" {
		return offset
	}
	offset += mtr.ReplayBfrOverflow.Size()

	if fldName == "Txbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_0Uncorrectable.Size()

	if fldName == "Txbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_1Uncorrectable.Size()

	if fldName == "Txbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_2Uncorrectable.Size()

	if fldName == "Txbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_3Uncorrectable.Size()

	if fldName == "Txbuf_0Correctable" {
		return offset
	}
	offset += mtr.Txbuf_0Correctable.Size()

	if fldName == "Txbuf_1Correctable" {
		return offset
	}
	offset += mtr.Txbuf_1Correctable.Size()

	if fldName == "Txbuf_2Correctable" {
		return offset
	}
	offset += mtr.Txbuf_2Correctable.Size()

	if fldName == "Txbuf_3Correctable" {
		return offset
	}
	offset += mtr.Txbuf_3Correctable.Size()

	return offset
}

// SetRxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportp0intpeccMetrics) SetRxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxbfrOverflow"))
	return nil
}

// SetReplayBfrOverflow sets cunter in shared memory
func (mtr *Ppppportp0intpeccMetrics) SetReplayBfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ReplayBfrOverflow"))
	return nil
}

// SetTxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp0intpeccMetrics) SetTxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Uncorrectable"))
	return nil
}

// SetTxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp0intpeccMetrics) SetTxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Uncorrectable"))
	return nil
}

// SetTxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp0intpeccMetrics) SetTxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Uncorrectable"))
	return nil
}

// SetTxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp0intpeccMetrics) SetTxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Uncorrectable"))
	return nil
}

// SetTxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportp0intpeccMetrics) SetTxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Correctable"))
	return nil
}

// SetTxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportp0intpeccMetrics) SetTxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Correctable"))
	return nil
}

// SetTxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportp0intpeccMetrics) SetTxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Correctable"))
	return nil
}

// SetTxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportp0intpeccMetrics) SetTxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Correctable"))
	return nil
}

// Ppppportp0intpeccMetricsIterator is the iterator object
type Ppppportp0intpeccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportp0intpeccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportp0intpeccMetricsIterator) Next() *Ppppportp0intpeccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportp0intpeccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportp0intpeccMetricsIterator) Find(key uint64) (*Ppppportp0intpeccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportp0intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportp0intpeccMetricsIterator) Create(key uint64) (*Ppppportp0intpeccMetrics, error) {
	tmtr := &Ppppportp0intpeccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportp0intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportp0intpeccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportp0intpeccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportp0intpeccMetricsIterator returns an iterator
func NewPpppportp0intpeccMetricsIterator() (*Ppppportp0intpeccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportp0intpeccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportp0intpeccMetricsIterator{iter: iter}, nil
}

type Ppppportp1intpeccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RxbfrOverflow metrics.Counter

	ReplayBfrOverflow metrics.Counter

	Txbuf_0Uncorrectable metrics.Counter

	Txbuf_1Uncorrectable metrics.Counter

	Txbuf_2Uncorrectable metrics.Counter

	Txbuf_3Uncorrectable metrics.Counter

	Txbuf_0Correctable metrics.Counter

	Txbuf_1Correctable metrics.Counter

	Txbuf_2Correctable metrics.Counter

	Txbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportp1intpeccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportp1intpeccMetrics) Size() int {
	sz := 0

	sz += mtr.RxbfrOverflow.Size()

	sz += mtr.ReplayBfrOverflow.Size()

	sz += mtr.Txbuf_0Uncorrectable.Size()

	sz += mtr.Txbuf_1Uncorrectable.Size()

	sz += mtr.Txbuf_2Uncorrectable.Size()

	sz += mtr.Txbuf_3Uncorrectable.Size()

	sz += mtr.Txbuf_0Correctable.Size()

	sz += mtr.Txbuf_1Correctable.Size()

	sz += mtr.Txbuf_2Correctable.Size()

	sz += mtr.Txbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportp1intpeccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.RxbfrOverflow.Size()

	mtr.ReplayBfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ReplayBfrOverflow.Size()

	mtr.Txbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Uncorrectable.Size()

	mtr.Txbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Uncorrectable.Size()

	mtr.Txbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Uncorrectable.Size()

	mtr.Txbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Uncorrectable.Size()

	mtr.Txbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Correctable.Size()

	mtr.Txbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Correctable.Size()

	mtr.Txbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Correctable.Size()

	mtr.Txbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportp1intpeccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxbfrOverflow" {
		return offset
	}
	offset += mtr.RxbfrOverflow.Size()

	if fldName == "ReplayBfrOverflow" {
		return offset
	}
	offset += mtr.ReplayBfrOverflow.Size()

	if fldName == "Txbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_0Uncorrectable.Size()

	if fldName == "Txbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_1Uncorrectable.Size()

	if fldName == "Txbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_2Uncorrectable.Size()

	if fldName == "Txbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_3Uncorrectable.Size()

	if fldName == "Txbuf_0Correctable" {
		return offset
	}
	offset += mtr.Txbuf_0Correctable.Size()

	if fldName == "Txbuf_1Correctable" {
		return offset
	}
	offset += mtr.Txbuf_1Correctable.Size()

	if fldName == "Txbuf_2Correctable" {
		return offset
	}
	offset += mtr.Txbuf_2Correctable.Size()

	if fldName == "Txbuf_3Correctable" {
		return offset
	}
	offset += mtr.Txbuf_3Correctable.Size()

	return offset
}

// SetRxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportp1intpeccMetrics) SetRxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxbfrOverflow"))
	return nil
}

// SetReplayBfrOverflow sets cunter in shared memory
func (mtr *Ppppportp1intpeccMetrics) SetReplayBfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ReplayBfrOverflow"))
	return nil
}

// SetTxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp1intpeccMetrics) SetTxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Uncorrectable"))
	return nil
}

// SetTxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp1intpeccMetrics) SetTxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Uncorrectable"))
	return nil
}

// SetTxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp1intpeccMetrics) SetTxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Uncorrectable"))
	return nil
}

// SetTxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp1intpeccMetrics) SetTxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Uncorrectable"))
	return nil
}

// SetTxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportp1intpeccMetrics) SetTxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Correctable"))
	return nil
}

// SetTxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportp1intpeccMetrics) SetTxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Correctable"))
	return nil
}

// SetTxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportp1intpeccMetrics) SetTxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Correctable"))
	return nil
}

// SetTxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportp1intpeccMetrics) SetTxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Correctable"))
	return nil
}

// Ppppportp1intpeccMetricsIterator is the iterator object
type Ppppportp1intpeccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportp1intpeccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportp1intpeccMetricsIterator) Next() *Ppppportp1intpeccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportp1intpeccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportp1intpeccMetricsIterator) Find(key uint64) (*Ppppportp1intpeccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportp1intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportp1intpeccMetricsIterator) Create(key uint64) (*Ppppportp1intpeccMetrics, error) {
	tmtr := &Ppppportp1intpeccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportp1intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportp1intpeccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportp1intpeccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportp1intpeccMetricsIterator returns an iterator
func NewPpppportp1intpeccMetricsIterator() (*Ppppportp1intpeccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportp1intpeccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportp1intpeccMetricsIterator{iter: iter}, nil
}

type Ppppportp2intpeccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RxbfrOverflow metrics.Counter

	ReplayBfrOverflow metrics.Counter

	Txbuf_0Uncorrectable metrics.Counter

	Txbuf_1Uncorrectable metrics.Counter

	Txbuf_2Uncorrectable metrics.Counter

	Txbuf_3Uncorrectable metrics.Counter

	Txbuf_0Correctable metrics.Counter

	Txbuf_1Correctable metrics.Counter

	Txbuf_2Correctable metrics.Counter

	Txbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportp2intpeccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportp2intpeccMetrics) Size() int {
	sz := 0

	sz += mtr.RxbfrOverflow.Size()

	sz += mtr.ReplayBfrOverflow.Size()

	sz += mtr.Txbuf_0Uncorrectable.Size()

	sz += mtr.Txbuf_1Uncorrectable.Size()

	sz += mtr.Txbuf_2Uncorrectable.Size()

	sz += mtr.Txbuf_3Uncorrectable.Size()

	sz += mtr.Txbuf_0Correctable.Size()

	sz += mtr.Txbuf_1Correctable.Size()

	sz += mtr.Txbuf_2Correctable.Size()

	sz += mtr.Txbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportp2intpeccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.RxbfrOverflow.Size()

	mtr.ReplayBfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ReplayBfrOverflow.Size()

	mtr.Txbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Uncorrectable.Size()

	mtr.Txbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Uncorrectable.Size()

	mtr.Txbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Uncorrectable.Size()

	mtr.Txbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Uncorrectable.Size()

	mtr.Txbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Correctable.Size()

	mtr.Txbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Correctable.Size()

	mtr.Txbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Correctable.Size()

	mtr.Txbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportp2intpeccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxbfrOverflow" {
		return offset
	}
	offset += mtr.RxbfrOverflow.Size()

	if fldName == "ReplayBfrOverflow" {
		return offset
	}
	offset += mtr.ReplayBfrOverflow.Size()

	if fldName == "Txbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_0Uncorrectable.Size()

	if fldName == "Txbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_1Uncorrectable.Size()

	if fldName == "Txbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_2Uncorrectable.Size()

	if fldName == "Txbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_3Uncorrectable.Size()

	if fldName == "Txbuf_0Correctable" {
		return offset
	}
	offset += mtr.Txbuf_0Correctable.Size()

	if fldName == "Txbuf_1Correctable" {
		return offset
	}
	offset += mtr.Txbuf_1Correctable.Size()

	if fldName == "Txbuf_2Correctable" {
		return offset
	}
	offset += mtr.Txbuf_2Correctable.Size()

	if fldName == "Txbuf_3Correctable" {
		return offset
	}
	offset += mtr.Txbuf_3Correctable.Size()

	return offset
}

// SetRxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportp2intpeccMetrics) SetRxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxbfrOverflow"))
	return nil
}

// SetReplayBfrOverflow sets cunter in shared memory
func (mtr *Ppppportp2intpeccMetrics) SetReplayBfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ReplayBfrOverflow"))
	return nil
}

// SetTxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp2intpeccMetrics) SetTxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Uncorrectable"))
	return nil
}

// SetTxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp2intpeccMetrics) SetTxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Uncorrectable"))
	return nil
}

// SetTxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp2intpeccMetrics) SetTxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Uncorrectable"))
	return nil
}

// SetTxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp2intpeccMetrics) SetTxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Uncorrectable"))
	return nil
}

// SetTxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportp2intpeccMetrics) SetTxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Correctable"))
	return nil
}

// SetTxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportp2intpeccMetrics) SetTxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Correctable"))
	return nil
}

// SetTxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportp2intpeccMetrics) SetTxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Correctable"))
	return nil
}

// SetTxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportp2intpeccMetrics) SetTxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Correctable"))
	return nil
}

// Ppppportp2intpeccMetricsIterator is the iterator object
type Ppppportp2intpeccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportp2intpeccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportp2intpeccMetricsIterator) Next() *Ppppportp2intpeccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportp2intpeccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportp2intpeccMetricsIterator) Find(key uint64) (*Ppppportp2intpeccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportp2intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportp2intpeccMetricsIterator) Create(key uint64) (*Ppppportp2intpeccMetrics, error) {
	tmtr := &Ppppportp2intpeccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportp2intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportp2intpeccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportp2intpeccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportp2intpeccMetricsIterator returns an iterator
func NewPpppportp2intpeccMetricsIterator() (*Ppppportp2intpeccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportp2intpeccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportp2intpeccMetricsIterator{iter: iter}, nil
}

type Ppppportp3intpeccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RxbfrOverflow metrics.Counter

	ReplayBfrOverflow metrics.Counter

	Txbuf_0Uncorrectable metrics.Counter

	Txbuf_1Uncorrectable metrics.Counter

	Txbuf_2Uncorrectable metrics.Counter

	Txbuf_3Uncorrectable metrics.Counter

	Txbuf_0Correctable metrics.Counter

	Txbuf_1Correctable metrics.Counter

	Txbuf_2Correctable metrics.Counter

	Txbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportp3intpeccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportp3intpeccMetrics) Size() int {
	sz := 0

	sz += mtr.RxbfrOverflow.Size()

	sz += mtr.ReplayBfrOverflow.Size()

	sz += mtr.Txbuf_0Uncorrectable.Size()

	sz += mtr.Txbuf_1Uncorrectable.Size()

	sz += mtr.Txbuf_2Uncorrectable.Size()

	sz += mtr.Txbuf_3Uncorrectable.Size()

	sz += mtr.Txbuf_0Correctable.Size()

	sz += mtr.Txbuf_1Correctable.Size()

	sz += mtr.Txbuf_2Correctable.Size()

	sz += mtr.Txbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportp3intpeccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.RxbfrOverflow.Size()

	mtr.ReplayBfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ReplayBfrOverflow.Size()

	mtr.Txbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Uncorrectable.Size()

	mtr.Txbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Uncorrectable.Size()

	mtr.Txbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Uncorrectable.Size()

	mtr.Txbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Uncorrectable.Size()

	mtr.Txbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Correctable.Size()

	mtr.Txbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Correctable.Size()

	mtr.Txbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Correctable.Size()

	mtr.Txbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportp3intpeccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxbfrOverflow" {
		return offset
	}
	offset += mtr.RxbfrOverflow.Size()

	if fldName == "ReplayBfrOverflow" {
		return offset
	}
	offset += mtr.ReplayBfrOverflow.Size()

	if fldName == "Txbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_0Uncorrectable.Size()

	if fldName == "Txbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_1Uncorrectable.Size()

	if fldName == "Txbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_2Uncorrectable.Size()

	if fldName == "Txbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_3Uncorrectable.Size()

	if fldName == "Txbuf_0Correctable" {
		return offset
	}
	offset += mtr.Txbuf_0Correctable.Size()

	if fldName == "Txbuf_1Correctable" {
		return offset
	}
	offset += mtr.Txbuf_1Correctable.Size()

	if fldName == "Txbuf_2Correctable" {
		return offset
	}
	offset += mtr.Txbuf_2Correctable.Size()

	if fldName == "Txbuf_3Correctable" {
		return offset
	}
	offset += mtr.Txbuf_3Correctable.Size()

	return offset
}

// SetRxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportp3intpeccMetrics) SetRxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxbfrOverflow"))
	return nil
}

// SetReplayBfrOverflow sets cunter in shared memory
func (mtr *Ppppportp3intpeccMetrics) SetReplayBfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ReplayBfrOverflow"))
	return nil
}

// SetTxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp3intpeccMetrics) SetTxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Uncorrectable"))
	return nil
}

// SetTxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp3intpeccMetrics) SetTxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Uncorrectable"))
	return nil
}

// SetTxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp3intpeccMetrics) SetTxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Uncorrectable"))
	return nil
}

// SetTxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp3intpeccMetrics) SetTxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Uncorrectable"))
	return nil
}

// SetTxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportp3intpeccMetrics) SetTxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Correctable"))
	return nil
}

// SetTxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportp3intpeccMetrics) SetTxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Correctable"))
	return nil
}

// SetTxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportp3intpeccMetrics) SetTxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Correctable"))
	return nil
}

// SetTxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportp3intpeccMetrics) SetTxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Correctable"))
	return nil
}

// Ppppportp3intpeccMetricsIterator is the iterator object
type Ppppportp3intpeccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportp3intpeccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportp3intpeccMetricsIterator) Next() *Ppppportp3intpeccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportp3intpeccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportp3intpeccMetricsIterator) Find(key uint64) (*Ppppportp3intpeccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportp3intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportp3intpeccMetricsIterator) Create(key uint64) (*Ppppportp3intpeccMetrics, error) {
	tmtr := &Ppppportp3intpeccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportp3intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportp3intpeccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportp3intpeccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportp3intpeccMetricsIterator returns an iterator
func NewPpppportp3intpeccMetricsIterator() (*Ppppportp3intpeccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportp3intpeccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportp3intpeccMetricsIterator{iter: iter}, nil
}

type Ppppportp4intpeccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RxbfrOverflow metrics.Counter

	ReplayBfrOverflow metrics.Counter

	Txbuf_0Uncorrectable metrics.Counter

	Txbuf_1Uncorrectable metrics.Counter

	Txbuf_2Uncorrectable metrics.Counter

	Txbuf_3Uncorrectable metrics.Counter

	Txbuf_0Correctable metrics.Counter

	Txbuf_1Correctable metrics.Counter

	Txbuf_2Correctable metrics.Counter

	Txbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportp4intpeccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportp4intpeccMetrics) Size() int {
	sz := 0

	sz += mtr.RxbfrOverflow.Size()

	sz += mtr.ReplayBfrOverflow.Size()

	sz += mtr.Txbuf_0Uncorrectable.Size()

	sz += mtr.Txbuf_1Uncorrectable.Size()

	sz += mtr.Txbuf_2Uncorrectable.Size()

	sz += mtr.Txbuf_3Uncorrectable.Size()

	sz += mtr.Txbuf_0Correctable.Size()

	sz += mtr.Txbuf_1Correctable.Size()

	sz += mtr.Txbuf_2Correctable.Size()

	sz += mtr.Txbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportp4intpeccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.RxbfrOverflow.Size()

	mtr.ReplayBfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ReplayBfrOverflow.Size()

	mtr.Txbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Uncorrectable.Size()

	mtr.Txbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Uncorrectable.Size()

	mtr.Txbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Uncorrectable.Size()

	mtr.Txbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Uncorrectable.Size()

	mtr.Txbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Correctable.Size()

	mtr.Txbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Correctable.Size()

	mtr.Txbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Correctable.Size()

	mtr.Txbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportp4intpeccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxbfrOverflow" {
		return offset
	}
	offset += mtr.RxbfrOverflow.Size()

	if fldName == "ReplayBfrOverflow" {
		return offset
	}
	offset += mtr.ReplayBfrOverflow.Size()

	if fldName == "Txbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_0Uncorrectable.Size()

	if fldName == "Txbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_1Uncorrectable.Size()

	if fldName == "Txbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_2Uncorrectable.Size()

	if fldName == "Txbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_3Uncorrectable.Size()

	if fldName == "Txbuf_0Correctable" {
		return offset
	}
	offset += mtr.Txbuf_0Correctable.Size()

	if fldName == "Txbuf_1Correctable" {
		return offset
	}
	offset += mtr.Txbuf_1Correctable.Size()

	if fldName == "Txbuf_2Correctable" {
		return offset
	}
	offset += mtr.Txbuf_2Correctable.Size()

	if fldName == "Txbuf_3Correctable" {
		return offset
	}
	offset += mtr.Txbuf_3Correctable.Size()

	return offset
}

// SetRxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportp4intpeccMetrics) SetRxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxbfrOverflow"))
	return nil
}

// SetReplayBfrOverflow sets cunter in shared memory
func (mtr *Ppppportp4intpeccMetrics) SetReplayBfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ReplayBfrOverflow"))
	return nil
}

// SetTxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp4intpeccMetrics) SetTxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Uncorrectable"))
	return nil
}

// SetTxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp4intpeccMetrics) SetTxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Uncorrectable"))
	return nil
}

// SetTxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp4intpeccMetrics) SetTxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Uncorrectable"))
	return nil
}

// SetTxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp4intpeccMetrics) SetTxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Uncorrectable"))
	return nil
}

// SetTxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportp4intpeccMetrics) SetTxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Correctable"))
	return nil
}

// SetTxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportp4intpeccMetrics) SetTxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Correctable"))
	return nil
}

// SetTxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportp4intpeccMetrics) SetTxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Correctable"))
	return nil
}

// SetTxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportp4intpeccMetrics) SetTxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Correctable"))
	return nil
}

// Ppppportp4intpeccMetricsIterator is the iterator object
type Ppppportp4intpeccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportp4intpeccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportp4intpeccMetricsIterator) Next() *Ppppportp4intpeccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportp4intpeccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportp4intpeccMetricsIterator) Find(key uint64) (*Ppppportp4intpeccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportp4intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportp4intpeccMetricsIterator) Create(key uint64) (*Ppppportp4intpeccMetrics, error) {
	tmtr := &Ppppportp4intpeccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportp4intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportp4intpeccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportp4intpeccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportp4intpeccMetricsIterator returns an iterator
func NewPpppportp4intpeccMetricsIterator() (*Ppppportp4intpeccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportp4intpeccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportp4intpeccMetricsIterator{iter: iter}, nil
}

type Ppppportp5intpeccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RxbfrOverflow metrics.Counter

	ReplayBfrOverflow metrics.Counter

	Txbuf_0Uncorrectable metrics.Counter

	Txbuf_1Uncorrectable metrics.Counter

	Txbuf_2Uncorrectable metrics.Counter

	Txbuf_3Uncorrectable metrics.Counter

	Txbuf_0Correctable metrics.Counter

	Txbuf_1Correctable metrics.Counter

	Txbuf_2Correctable metrics.Counter

	Txbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportp5intpeccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportp5intpeccMetrics) Size() int {
	sz := 0

	sz += mtr.RxbfrOverflow.Size()

	sz += mtr.ReplayBfrOverflow.Size()

	sz += mtr.Txbuf_0Uncorrectable.Size()

	sz += mtr.Txbuf_1Uncorrectable.Size()

	sz += mtr.Txbuf_2Uncorrectable.Size()

	sz += mtr.Txbuf_3Uncorrectable.Size()

	sz += mtr.Txbuf_0Correctable.Size()

	sz += mtr.Txbuf_1Correctable.Size()

	sz += mtr.Txbuf_2Correctable.Size()

	sz += mtr.Txbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportp5intpeccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.RxbfrOverflow.Size()

	mtr.ReplayBfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ReplayBfrOverflow.Size()

	mtr.Txbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Uncorrectable.Size()

	mtr.Txbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Uncorrectable.Size()

	mtr.Txbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Uncorrectable.Size()

	mtr.Txbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Uncorrectable.Size()

	mtr.Txbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Correctable.Size()

	mtr.Txbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Correctable.Size()

	mtr.Txbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Correctable.Size()

	mtr.Txbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportp5intpeccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxbfrOverflow" {
		return offset
	}
	offset += mtr.RxbfrOverflow.Size()

	if fldName == "ReplayBfrOverflow" {
		return offset
	}
	offset += mtr.ReplayBfrOverflow.Size()

	if fldName == "Txbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_0Uncorrectable.Size()

	if fldName == "Txbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_1Uncorrectable.Size()

	if fldName == "Txbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_2Uncorrectable.Size()

	if fldName == "Txbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_3Uncorrectable.Size()

	if fldName == "Txbuf_0Correctable" {
		return offset
	}
	offset += mtr.Txbuf_0Correctable.Size()

	if fldName == "Txbuf_1Correctable" {
		return offset
	}
	offset += mtr.Txbuf_1Correctable.Size()

	if fldName == "Txbuf_2Correctable" {
		return offset
	}
	offset += mtr.Txbuf_2Correctable.Size()

	if fldName == "Txbuf_3Correctable" {
		return offset
	}
	offset += mtr.Txbuf_3Correctable.Size()

	return offset
}

// SetRxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportp5intpeccMetrics) SetRxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxbfrOverflow"))
	return nil
}

// SetReplayBfrOverflow sets cunter in shared memory
func (mtr *Ppppportp5intpeccMetrics) SetReplayBfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ReplayBfrOverflow"))
	return nil
}

// SetTxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp5intpeccMetrics) SetTxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Uncorrectable"))
	return nil
}

// SetTxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp5intpeccMetrics) SetTxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Uncorrectable"))
	return nil
}

// SetTxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp5intpeccMetrics) SetTxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Uncorrectable"))
	return nil
}

// SetTxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp5intpeccMetrics) SetTxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Uncorrectable"))
	return nil
}

// SetTxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportp5intpeccMetrics) SetTxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Correctable"))
	return nil
}

// SetTxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportp5intpeccMetrics) SetTxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Correctable"))
	return nil
}

// SetTxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportp5intpeccMetrics) SetTxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Correctable"))
	return nil
}

// SetTxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportp5intpeccMetrics) SetTxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Correctable"))
	return nil
}

// Ppppportp5intpeccMetricsIterator is the iterator object
type Ppppportp5intpeccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportp5intpeccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportp5intpeccMetricsIterator) Next() *Ppppportp5intpeccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportp5intpeccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportp5intpeccMetricsIterator) Find(key uint64) (*Ppppportp5intpeccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportp5intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportp5intpeccMetricsIterator) Create(key uint64) (*Ppppportp5intpeccMetrics, error) {
	tmtr := &Ppppportp5intpeccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportp5intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportp5intpeccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportp5intpeccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportp5intpeccMetricsIterator returns an iterator
func NewPpppportp5intpeccMetricsIterator() (*Ppppportp5intpeccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportp5intpeccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportp5intpeccMetricsIterator{iter: iter}, nil
}

type Ppppportp6intpeccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RxbfrOverflow metrics.Counter

	ReplayBfrOverflow metrics.Counter

	Txbuf_0Uncorrectable metrics.Counter

	Txbuf_1Uncorrectable metrics.Counter

	Txbuf_2Uncorrectable metrics.Counter

	Txbuf_3Uncorrectable metrics.Counter

	Txbuf_0Correctable metrics.Counter

	Txbuf_1Correctable metrics.Counter

	Txbuf_2Correctable metrics.Counter

	Txbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportp6intpeccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportp6intpeccMetrics) Size() int {
	sz := 0

	sz += mtr.RxbfrOverflow.Size()

	sz += mtr.ReplayBfrOverflow.Size()

	sz += mtr.Txbuf_0Uncorrectable.Size()

	sz += mtr.Txbuf_1Uncorrectable.Size()

	sz += mtr.Txbuf_2Uncorrectable.Size()

	sz += mtr.Txbuf_3Uncorrectable.Size()

	sz += mtr.Txbuf_0Correctable.Size()

	sz += mtr.Txbuf_1Correctable.Size()

	sz += mtr.Txbuf_2Correctable.Size()

	sz += mtr.Txbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportp6intpeccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.RxbfrOverflow.Size()

	mtr.ReplayBfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ReplayBfrOverflow.Size()

	mtr.Txbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Uncorrectable.Size()

	mtr.Txbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Uncorrectable.Size()

	mtr.Txbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Uncorrectable.Size()

	mtr.Txbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Uncorrectable.Size()

	mtr.Txbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Correctable.Size()

	mtr.Txbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Correctable.Size()

	mtr.Txbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Correctable.Size()

	mtr.Txbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportp6intpeccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxbfrOverflow" {
		return offset
	}
	offset += mtr.RxbfrOverflow.Size()

	if fldName == "ReplayBfrOverflow" {
		return offset
	}
	offset += mtr.ReplayBfrOverflow.Size()

	if fldName == "Txbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_0Uncorrectable.Size()

	if fldName == "Txbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_1Uncorrectable.Size()

	if fldName == "Txbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_2Uncorrectable.Size()

	if fldName == "Txbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_3Uncorrectable.Size()

	if fldName == "Txbuf_0Correctable" {
		return offset
	}
	offset += mtr.Txbuf_0Correctable.Size()

	if fldName == "Txbuf_1Correctable" {
		return offset
	}
	offset += mtr.Txbuf_1Correctable.Size()

	if fldName == "Txbuf_2Correctable" {
		return offset
	}
	offset += mtr.Txbuf_2Correctable.Size()

	if fldName == "Txbuf_3Correctable" {
		return offset
	}
	offset += mtr.Txbuf_3Correctable.Size()

	return offset
}

// SetRxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportp6intpeccMetrics) SetRxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxbfrOverflow"))
	return nil
}

// SetReplayBfrOverflow sets cunter in shared memory
func (mtr *Ppppportp6intpeccMetrics) SetReplayBfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ReplayBfrOverflow"))
	return nil
}

// SetTxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp6intpeccMetrics) SetTxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Uncorrectable"))
	return nil
}

// SetTxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp6intpeccMetrics) SetTxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Uncorrectable"))
	return nil
}

// SetTxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp6intpeccMetrics) SetTxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Uncorrectable"))
	return nil
}

// SetTxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp6intpeccMetrics) SetTxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Uncorrectable"))
	return nil
}

// SetTxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportp6intpeccMetrics) SetTxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Correctable"))
	return nil
}

// SetTxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportp6intpeccMetrics) SetTxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Correctable"))
	return nil
}

// SetTxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportp6intpeccMetrics) SetTxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Correctable"))
	return nil
}

// SetTxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportp6intpeccMetrics) SetTxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Correctable"))
	return nil
}

// Ppppportp6intpeccMetricsIterator is the iterator object
type Ppppportp6intpeccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportp6intpeccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportp6intpeccMetricsIterator) Next() *Ppppportp6intpeccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportp6intpeccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportp6intpeccMetricsIterator) Find(key uint64) (*Ppppportp6intpeccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportp6intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportp6intpeccMetricsIterator) Create(key uint64) (*Ppppportp6intpeccMetrics, error) {
	tmtr := &Ppppportp6intpeccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportp6intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportp6intpeccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportp6intpeccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportp6intpeccMetricsIterator returns an iterator
func NewPpppportp6intpeccMetricsIterator() (*Ppppportp6intpeccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportp6intpeccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportp6intpeccMetricsIterator{iter: iter}, nil
}

type Ppppportp7intpeccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RxbfrOverflow metrics.Counter

	ReplayBfrOverflow metrics.Counter

	Txbuf_0Uncorrectable metrics.Counter

	Txbuf_1Uncorrectable metrics.Counter

	Txbuf_2Uncorrectable metrics.Counter

	Txbuf_3Uncorrectable metrics.Counter

	Txbuf_0Correctable metrics.Counter

	Txbuf_1Correctable metrics.Counter

	Txbuf_2Correctable metrics.Counter

	Txbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportp7intpeccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportp7intpeccMetrics) Size() int {
	sz := 0

	sz += mtr.RxbfrOverflow.Size()

	sz += mtr.ReplayBfrOverflow.Size()

	sz += mtr.Txbuf_0Uncorrectable.Size()

	sz += mtr.Txbuf_1Uncorrectable.Size()

	sz += mtr.Txbuf_2Uncorrectable.Size()

	sz += mtr.Txbuf_3Uncorrectable.Size()

	sz += mtr.Txbuf_0Correctable.Size()

	sz += mtr.Txbuf_1Correctable.Size()

	sz += mtr.Txbuf_2Correctable.Size()

	sz += mtr.Txbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportp7intpeccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.RxbfrOverflow.Size()

	mtr.ReplayBfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ReplayBfrOverflow.Size()

	mtr.Txbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Uncorrectable.Size()

	mtr.Txbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Uncorrectable.Size()

	mtr.Txbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Uncorrectable.Size()

	mtr.Txbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Uncorrectable.Size()

	mtr.Txbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_0Correctable.Size()

	mtr.Txbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_1Correctable.Size()

	mtr.Txbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_2Correctable.Size()

	mtr.Txbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Txbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportp7intpeccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxbfrOverflow" {
		return offset
	}
	offset += mtr.RxbfrOverflow.Size()

	if fldName == "ReplayBfrOverflow" {
		return offset
	}
	offset += mtr.ReplayBfrOverflow.Size()

	if fldName == "Txbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_0Uncorrectable.Size()

	if fldName == "Txbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_1Uncorrectable.Size()

	if fldName == "Txbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_2Uncorrectable.Size()

	if fldName == "Txbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Txbuf_3Uncorrectable.Size()

	if fldName == "Txbuf_0Correctable" {
		return offset
	}
	offset += mtr.Txbuf_0Correctable.Size()

	if fldName == "Txbuf_1Correctable" {
		return offset
	}
	offset += mtr.Txbuf_1Correctable.Size()

	if fldName == "Txbuf_2Correctable" {
		return offset
	}
	offset += mtr.Txbuf_2Correctable.Size()

	if fldName == "Txbuf_3Correctable" {
		return offset
	}
	offset += mtr.Txbuf_3Correctable.Size()

	return offset
}

// SetRxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportp7intpeccMetrics) SetRxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxbfrOverflow"))
	return nil
}

// SetReplayBfrOverflow sets cunter in shared memory
func (mtr *Ppppportp7intpeccMetrics) SetReplayBfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ReplayBfrOverflow"))
	return nil
}

// SetTxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp7intpeccMetrics) SetTxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Uncorrectable"))
	return nil
}

// SetTxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp7intpeccMetrics) SetTxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Uncorrectable"))
	return nil
}

// SetTxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp7intpeccMetrics) SetTxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Uncorrectable"))
	return nil
}

// SetTxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportp7intpeccMetrics) SetTxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Uncorrectable"))
	return nil
}

// SetTxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportp7intpeccMetrics) SetTxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_0Correctable"))
	return nil
}

// SetTxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportp7intpeccMetrics) SetTxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_1Correctable"))
	return nil
}

// SetTxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportp7intpeccMetrics) SetTxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_2Correctable"))
	return nil
}

// SetTxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportp7intpeccMetrics) SetTxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Txbuf_3Correctable"))
	return nil
}

// Ppppportp7intpeccMetricsIterator is the iterator object
type Ppppportp7intpeccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportp7intpeccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportp7intpeccMetricsIterator) Next() *Ppppportp7intpeccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportp7intpeccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportp7intpeccMetricsIterator) Find(key uint64) (*Ppppportp7intpeccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportp7intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportp7intpeccMetricsIterator) Create(key uint64) (*Ppppportp7intpeccMetrics, error) {
	tmtr := &Ppppportp7intpeccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportp7intpeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportp7intpeccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportp7intpeccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportp7intpeccMetricsIterator returns an iterator
func NewPpppportp7intpeccMetricsIterator() (*Ppppportp7intpeccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportp7intpeccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportp7intpeccMetricsIterator{iter: iter}, nil
}

type Ppppportc0intcmacMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	TxbfrOverflow metrics.Counter

	RxtlpErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc0intcmacMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc0intcmacMetrics) Size() int {
	sz := 0

	sz += mtr.TxbfrOverflow.Size()

	sz += mtr.RxtlpErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc0intcmacMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.TxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.TxbfrOverflow.Size()

	mtr.RxtlpErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RxtlpErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc0intcmacMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "TxbfrOverflow" {
		return offset
	}
	offset += mtr.TxbfrOverflow.Size()

	if fldName == "RxtlpErr" {
		return offset
	}
	offset += mtr.RxtlpErr.Size()

	return offset
}

// SetTxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportc0intcmacMetrics) SetTxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxbfrOverflow"))
	return nil
}

// SetRxtlpErr sets cunter in shared memory
func (mtr *Ppppportc0intcmacMetrics) SetRxtlpErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxtlpErr"))
	return nil
}

// Ppppportc0intcmacMetricsIterator is the iterator object
type Ppppportc0intcmacMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc0intcmacMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc0intcmacMetricsIterator) Next() *Ppppportc0intcmacMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc0intcmacMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc0intcmacMetricsIterator) Find(key uint64) (*Ppppportc0intcmacMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc0intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc0intcmacMetricsIterator) Create(key uint64) (*Ppppportc0intcmacMetrics, error) {
	tmtr := &Ppppportc0intcmacMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc0intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc0intcmacMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc0intcmacMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc0intcmacMetricsIterator returns an iterator
func NewPpppportc0intcmacMetricsIterator() (*Ppppportc0intcmacMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc0intcmacMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc0intcmacMetricsIterator{iter: iter}, nil
}

type Ppppportc0intceccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Rxbuf_0Uncorrectable metrics.Counter

	Rxbuf_1Uncorrectable metrics.Counter

	Rxbuf_2Uncorrectable metrics.Counter

	Rxbuf_3Uncorrectable metrics.Counter

	Rxbuf_0Correctable metrics.Counter

	Rxbuf_1Correctable metrics.Counter

	Rxbuf_2Correctable metrics.Counter

	Rxbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc0intceccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc0intceccMetrics) Size() int {
	sz := 0

	sz += mtr.Rxbuf_0Uncorrectable.Size()

	sz += mtr.Rxbuf_1Uncorrectable.Size()

	sz += mtr.Rxbuf_2Uncorrectable.Size()

	sz += mtr.Rxbuf_3Uncorrectable.Size()

	sz += mtr.Rxbuf_0Correctable.Size()

	sz += mtr.Rxbuf_1Correctable.Size()

	sz += mtr.Rxbuf_2Correctable.Size()

	sz += mtr.Rxbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc0intceccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Rxbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	mtr.Rxbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	mtr.Rxbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	mtr.Rxbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	mtr.Rxbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Correctable.Size()

	mtr.Rxbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Correctable.Size()

	mtr.Rxbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Correctable.Size()

	mtr.Rxbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc0intceccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Rxbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	if fldName == "Rxbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	if fldName == "Rxbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	if fldName == "Rxbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	if fldName == "Rxbuf_0Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_0Correctable.Size()

	if fldName == "Rxbuf_1Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_1Correctable.Size()

	if fldName == "Rxbuf_2Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_2Correctable.Size()

	if fldName == "Rxbuf_3Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_3Correctable.Size()

	return offset
}

// SetRxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc0intceccMetrics) SetRxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Uncorrectable"))
	return nil
}

// SetRxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc0intceccMetrics) SetRxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Uncorrectable"))
	return nil
}

// SetRxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc0intceccMetrics) SetRxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Uncorrectable"))
	return nil
}

// SetRxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc0intceccMetrics) SetRxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Uncorrectable"))
	return nil
}

// SetRxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportc0intceccMetrics) SetRxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Correctable"))
	return nil
}

// SetRxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportc0intceccMetrics) SetRxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Correctable"))
	return nil
}

// SetRxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportc0intceccMetrics) SetRxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Correctable"))
	return nil
}

// SetRxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportc0intceccMetrics) SetRxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Correctable"))
	return nil
}

// Ppppportc0intceccMetricsIterator is the iterator object
type Ppppportc0intceccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc0intceccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc0intceccMetricsIterator) Next() *Ppppportc0intceccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc0intceccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc0intceccMetricsIterator) Find(key uint64) (*Ppppportc0intceccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc0intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc0intceccMetricsIterator) Create(key uint64) (*Ppppportc0intceccMetrics, error) {
	tmtr := &Ppppportc0intceccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc0intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc0intceccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc0intceccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc0intceccMetricsIterator returns an iterator
func NewPpppportc0intceccMetricsIterator() (*Ppppportc0intceccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc0intceccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc0intceccMetricsIterator{iter: iter}, nil
}

type Ppppportc1intcmacMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	TxbfrOverflow metrics.Counter

	RxtlpErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc1intcmacMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc1intcmacMetrics) Size() int {
	sz := 0

	sz += mtr.TxbfrOverflow.Size()

	sz += mtr.RxtlpErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc1intcmacMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.TxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.TxbfrOverflow.Size()

	mtr.RxtlpErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RxtlpErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc1intcmacMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "TxbfrOverflow" {
		return offset
	}
	offset += mtr.TxbfrOverflow.Size()

	if fldName == "RxtlpErr" {
		return offset
	}
	offset += mtr.RxtlpErr.Size()

	return offset
}

// SetTxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportc1intcmacMetrics) SetTxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxbfrOverflow"))
	return nil
}

// SetRxtlpErr sets cunter in shared memory
func (mtr *Ppppportc1intcmacMetrics) SetRxtlpErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxtlpErr"))
	return nil
}

// Ppppportc1intcmacMetricsIterator is the iterator object
type Ppppportc1intcmacMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc1intcmacMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc1intcmacMetricsIterator) Next() *Ppppportc1intcmacMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc1intcmacMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc1intcmacMetricsIterator) Find(key uint64) (*Ppppportc1intcmacMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc1intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc1intcmacMetricsIterator) Create(key uint64) (*Ppppportc1intcmacMetrics, error) {
	tmtr := &Ppppportc1intcmacMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc1intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc1intcmacMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc1intcmacMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc1intcmacMetricsIterator returns an iterator
func NewPpppportc1intcmacMetricsIterator() (*Ppppportc1intcmacMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc1intcmacMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc1intcmacMetricsIterator{iter: iter}, nil
}

type Ppppportc1intceccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Rxbuf_0Uncorrectable metrics.Counter

	Rxbuf_1Uncorrectable metrics.Counter

	Rxbuf_2Uncorrectable metrics.Counter

	Rxbuf_3Uncorrectable metrics.Counter

	Rxbuf_0Correctable metrics.Counter

	Rxbuf_1Correctable metrics.Counter

	Rxbuf_2Correctable metrics.Counter

	Rxbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc1intceccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc1intceccMetrics) Size() int {
	sz := 0

	sz += mtr.Rxbuf_0Uncorrectable.Size()

	sz += mtr.Rxbuf_1Uncorrectable.Size()

	sz += mtr.Rxbuf_2Uncorrectable.Size()

	sz += mtr.Rxbuf_3Uncorrectable.Size()

	sz += mtr.Rxbuf_0Correctable.Size()

	sz += mtr.Rxbuf_1Correctable.Size()

	sz += mtr.Rxbuf_2Correctable.Size()

	sz += mtr.Rxbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc1intceccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Rxbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	mtr.Rxbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	mtr.Rxbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	mtr.Rxbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	mtr.Rxbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Correctable.Size()

	mtr.Rxbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Correctable.Size()

	mtr.Rxbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Correctable.Size()

	mtr.Rxbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc1intceccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Rxbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	if fldName == "Rxbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	if fldName == "Rxbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	if fldName == "Rxbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	if fldName == "Rxbuf_0Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_0Correctable.Size()

	if fldName == "Rxbuf_1Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_1Correctable.Size()

	if fldName == "Rxbuf_2Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_2Correctable.Size()

	if fldName == "Rxbuf_3Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_3Correctable.Size()

	return offset
}

// SetRxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc1intceccMetrics) SetRxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Uncorrectable"))
	return nil
}

// SetRxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc1intceccMetrics) SetRxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Uncorrectable"))
	return nil
}

// SetRxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc1intceccMetrics) SetRxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Uncorrectable"))
	return nil
}

// SetRxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc1intceccMetrics) SetRxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Uncorrectable"))
	return nil
}

// SetRxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportc1intceccMetrics) SetRxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Correctable"))
	return nil
}

// SetRxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportc1intceccMetrics) SetRxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Correctable"))
	return nil
}

// SetRxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportc1intceccMetrics) SetRxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Correctable"))
	return nil
}

// SetRxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportc1intceccMetrics) SetRxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Correctable"))
	return nil
}

// Ppppportc1intceccMetricsIterator is the iterator object
type Ppppportc1intceccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc1intceccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc1intceccMetricsIterator) Next() *Ppppportc1intceccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc1intceccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc1intceccMetricsIterator) Find(key uint64) (*Ppppportc1intceccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc1intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc1intceccMetricsIterator) Create(key uint64) (*Ppppportc1intceccMetrics, error) {
	tmtr := &Ppppportc1intceccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc1intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc1intceccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc1intceccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc1intceccMetricsIterator returns an iterator
func NewPpppportc1intceccMetricsIterator() (*Ppppportc1intceccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc1intceccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc1intceccMetricsIterator{iter: iter}, nil
}

type Ppppportc2intcmacMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	TxbfrOverflow metrics.Counter

	RxtlpErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc2intcmacMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc2intcmacMetrics) Size() int {
	sz := 0

	sz += mtr.TxbfrOverflow.Size()

	sz += mtr.RxtlpErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc2intcmacMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.TxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.TxbfrOverflow.Size()

	mtr.RxtlpErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RxtlpErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc2intcmacMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "TxbfrOverflow" {
		return offset
	}
	offset += mtr.TxbfrOverflow.Size()

	if fldName == "RxtlpErr" {
		return offset
	}
	offset += mtr.RxtlpErr.Size()

	return offset
}

// SetTxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportc2intcmacMetrics) SetTxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxbfrOverflow"))
	return nil
}

// SetRxtlpErr sets cunter in shared memory
func (mtr *Ppppportc2intcmacMetrics) SetRxtlpErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxtlpErr"))
	return nil
}

// Ppppportc2intcmacMetricsIterator is the iterator object
type Ppppportc2intcmacMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc2intcmacMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc2intcmacMetricsIterator) Next() *Ppppportc2intcmacMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc2intcmacMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc2intcmacMetricsIterator) Find(key uint64) (*Ppppportc2intcmacMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc2intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc2intcmacMetricsIterator) Create(key uint64) (*Ppppportc2intcmacMetrics, error) {
	tmtr := &Ppppportc2intcmacMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc2intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc2intcmacMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc2intcmacMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc2intcmacMetricsIterator returns an iterator
func NewPpppportc2intcmacMetricsIterator() (*Ppppportc2intcmacMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc2intcmacMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc2intcmacMetricsIterator{iter: iter}, nil
}

type Ppppportc2intceccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Rxbuf_0Uncorrectable metrics.Counter

	Rxbuf_1Uncorrectable metrics.Counter

	Rxbuf_2Uncorrectable metrics.Counter

	Rxbuf_3Uncorrectable metrics.Counter

	Rxbuf_0Correctable metrics.Counter

	Rxbuf_1Correctable metrics.Counter

	Rxbuf_2Correctable metrics.Counter

	Rxbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc2intceccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc2intceccMetrics) Size() int {
	sz := 0

	sz += mtr.Rxbuf_0Uncorrectable.Size()

	sz += mtr.Rxbuf_1Uncorrectable.Size()

	sz += mtr.Rxbuf_2Uncorrectable.Size()

	sz += mtr.Rxbuf_3Uncorrectable.Size()

	sz += mtr.Rxbuf_0Correctable.Size()

	sz += mtr.Rxbuf_1Correctable.Size()

	sz += mtr.Rxbuf_2Correctable.Size()

	sz += mtr.Rxbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc2intceccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Rxbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	mtr.Rxbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	mtr.Rxbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	mtr.Rxbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	mtr.Rxbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Correctable.Size()

	mtr.Rxbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Correctable.Size()

	mtr.Rxbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Correctable.Size()

	mtr.Rxbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc2intceccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Rxbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	if fldName == "Rxbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	if fldName == "Rxbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	if fldName == "Rxbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	if fldName == "Rxbuf_0Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_0Correctable.Size()

	if fldName == "Rxbuf_1Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_1Correctable.Size()

	if fldName == "Rxbuf_2Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_2Correctable.Size()

	if fldName == "Rxbuf_3Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_3Correctable.Size()

	return offset
}

// SetRxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc2intceccMetrics) SetRxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Uncorrectable"))
	return nil
}

// SetRxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc2intceccMetrics) SetRxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Uncorrectable"))
	return nil
}

// SetRxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc2intceccMetrics) SetRxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Uncorrectable"))
	return nil
}

// SetRxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc2intceccMetrics) SetRxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Uncorrectable"))
	return nil
}

// SetRxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportc2intceccMetrics) SetRxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Correctable"))
	return nil
}

// SetRxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportc2intceccMetrics) SetRxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Correctable"))
	return nil
}

// SetRxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportc2intceccMetrics) SetRxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Correctable"))
	return nil
}

// SetRxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportc2intceccMetrics) SetRxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Correctable"))
	return nil
}

// Ppppportc2intceccMetricsIterator is the iterator object
type Ppppportc2intceccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc2intceccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc2intceccMetricsIterator) Next() *Ppppportc2intceccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc2intceccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc2intceccMetricsIterator) Find(key uint64) (*Ppppportc2intceccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc2intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc2intceccMetricsIterator) Create(key uint64) (*Ppppportc2intceccMetrics, error) {
	tmtr := &Ppppportc2intceccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc2intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc2intceccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc2intceccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc2intceccMetricsIterator returns an iterator
func NewPpppportc2intceccMetricsIterator() (*Ppppportc2intceccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc2intceccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc2intceccMetricsIterator{iter: iter}, nil
}

type Ppppportc3intcmacMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	TxbfrOverflow metrics.Counter

	RxtlpErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc3intcmacMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc3intcmacMetrics) Size() int {
	sz := 0

	sz += mtr.TxbfrOverflow.Size()

	sz += mtr.RxtlpErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc3intcmacMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.TxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.TxbfrOverflow.Size()

	mtr.RxtlpErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RxtlpErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc3intcmacMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "TxbfrOverflow" {
		return offset
	}
	offset += mtr.TxbfrOverflow.Size()

	if fldName == "RxtlpErr" {
		return offset
	}
	offset += mtr.RxtlpErr.Size()

	return offset
}

// SetTxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportc3intcmacMetrics) SetTxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxbfrOverflow"))
	return nil
}

// SetRxtlpErr sets cunter in shared memory
func (mtr *Ppppportc3intcmacMetrics) SetRxtlpErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxtlpErr"))
	return nil
}

// Ppppportc3intcmacMetricsIterator is the iterator object
type Ppppportc3intcmacMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc3intcmacMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc3intcmacMetricsIterator) Next() *Ppppportc3intcmacMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc3intcmacMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc3intcmacMetricsIterator) Find(key uint64) (*Ppppportc3intcmacMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc3intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc3intcmacMetricsIterator) Create(key uint64) (*Ppppportc3intcmacMetrics, error) {
	tmtr := &Ppppportc3intcmacMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc3intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc3intcmacMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc3intcmacMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc3intcmacMetricsIterator returns an iterator
func NewPpppportc3intcmacMetricsIterator() (*Ppppportc3intcmacMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc3intcmacMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc3intcmacMetricsIterator{iter: iter}, nil
}

type Ppppportc3intceccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Rxbuf_0Uncorrectable metrics.Counter

	Rxbuf_1Uncorrectable metrics.Counter

	Rxbuf_2Uncorrectable metrics.Counter

	Rxbuf_3Uncorrectable metrics.Counter

	Rxbuf_0Correctable metrics.Counter

	Rxbuf_1Correctable metrics.Counter

	Rxbuf_2Correctable metrics.Counter

	Rxbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc3intceccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc3intceccMetrics) Size() int {
	sz := 0

	sz += mtr.Rxbuf_0Uncorrectable.Size()

	sz += mtr.Rxbuf_1Uncorrectable.Size()

	sz += mtr.Rxbuf_2Uncorrectable.Size()

	sz += mtr.Rxbuf_3Uncorrectable.Size()

	sz += mtr.Rxbuf_0Correctable.Size()

	sz += mtr.Rxbuf_1Correctable.Size()

	sz += mtr.Rxbuf_2Correctable.Size()

	sz += mtr.Rxbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc3intceccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Rxbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	mtr.Rxbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	mtr.Rxbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	mtr.Rxbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	mtr.Rxbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Correctable.Size()

	mtr.Rxbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Correctable.Size()

	mtr.Rxbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Correctable.Size()

	mtr.Rxbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc3intceccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Rxbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	if fldName == "Rxbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	if fldName == "Rxbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	if fldName == "Rxbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	if fldName == "Rxbuf_0Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_0Correctable.Size()

	if fldName == "Rxbuf_1Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_1Correctable.Size()

	if fldName == "Rxbuf_2Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_2Correctable.Size()

	if fldName == "Rxbuf_3Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_3Correctable.Size()

	return offset
}

// SetRxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc3intceccMetrics) SetRxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Uncorrectable"))
	return nil
}

// SetRxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc3intceccMetrics) SetRxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Uncorrectable"))
	return nil
}

// SetRxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc3intceccMetrics) SetRxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Uncorrectable"))
	return nil
}

// SetRxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc3intceccMetrics) SetRxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Uncorrectable"))
	return nil
}

// SetRxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportc3intceccMetrics) SetRxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Correctable"))
	return nil
}

// SetRxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportc3intceccMetrics) SetRxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Correctable"))
	return nil
}

// SetRxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportc3intceccMetrics) SetRxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Correctable"))
	return nil
}

// SetRxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportc3intceccMetrics) SetRxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Correctable"))
	return nil
}

// Ppppportc3intceccMetricsIterator is the iterator object
type Ppppportc3intceccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc3intceccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc3intceccMetricsIterator) Next() *Ppppportc3intceccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc3intceccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc3intceccMetricsIterator) Find(key uint64) (*Ppppportc3intceccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc3intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc3intceccMetricsIterator) Create(key uint64) (*Ppppportc3intceccMetrics, error) {
	tmtr := &Ppppportc3intceccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc3intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc3intceccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc3intceccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc3intceccMetricsIterator returns an iterator
func NewPpppportc3intceccMetricsIterator() (*Ppppportc3intceccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc3intceccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc3intceccMetricsIterator{iter: iter}, nil
}

type Ppppportc4intcmacMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	TxbfrOverflow metrics.Counter

	RxtlpErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc4intcmacMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc4intcmacMetrics) Size() int {
	sz := 0

	sz += mtr.TxbfrOverflow.Size()

	sz += mtr.RxtlpErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc4intcmacMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.TxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.TxbfrOverflow.Size()

	mtr.RxtlpErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RxtlpErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc4intcmacMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "TxbfrOverflow" {
		return offset
	}
	offset += mtr.TxbfrOverflow.Size()

	if fldName == "RxtlpErr" {
		return offset
	}
	offset += mtr.RxtlpErr.Size()

	return offset
}

// SetTxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportc4intcmacMetrics) SetTxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxbfrOverflow"))
	return nil
}

// SetRxtlpErr sets cunter in shared memory
func (mtr *Ppppportc4intcmacMetrics) SetRxtlpErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxtlpErr"))
	return nil
}

// Ppppportc4intcmacMetricsIterator is the iterator object
type Ppppportc4intcmacMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc4intcmacMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc4intcmacMetricsIterator) Next() *Ppppportc4intcmacMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc4intcmacMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc4intcmacMetricsIterator) Find(key uint64) (*Ppppportc4intcmacMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc4intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc4intcmacMetricsIterator) Create(key uint64) (*Ppppportc4intcmacMetrics, error) {
	tmtr := &Ppppportc4intcmacMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc4intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc4intcmacMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc4intcmacMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc4intcmacMetricsIterator returns an iterator
func NewPpppportc4intcmacMetricsIterator() (*Ppppportc4intcmacMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc4intcmacMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc4intcmacMetricsIterator{iter: iter}, nil
}

type Ppppportc4intceccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Rxbuf_0Uncorrectable metrics.Counter

	Rxbuf_1Uncorrectable metrics.Counter

	Rxbuf_2Uncorrectable metrics.Counter

	Rxbuf_3Uncorrectable metrics.Counter

	Rxbuf_0Correctable metrics.Counter

	Rxbuf_1Correctable metrics.Counter

	Rxbuf_2Correctable metrics.Counter

	Rxbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc4intceccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc4intceccMetrics) Size() int {
	sz := 0

	sz += mtr.Rxbuf_0Uncorrectable.Size()

	sz += mtr.Rxbuf_1Uncorrectable.Size()

	sz += mtr.Rxbuf_2Uncorrectable.Size()

	sz += mtr.Rxbuf_3Uncorrectable.Size()

	sz += mtr.Rxbuf_0Correctable.Size()

	sz += mtr.Rxbuf_1Correctable.Size()

	sz += mtr.Rxbuf_2Correctable.Size()

	sz += mtr.Rxbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc4intceccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Rxbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	mtr.Rxbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	mtr.Rxbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	mtr.Rxbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	mtr.Rxbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Correctable.Size()

	mtr.Rxbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Correctable.Size()

	mtr.Rxbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Correctable.Size()

	mtr.Rxbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc4intceccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Rxbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	if fldName == "Rxbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	if fldName == "Rxbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	if fldName == "Rxbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	if fldName == "Rxbuf_0Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_0Correctable.Size()

	if fldName == "Rxbuf_1Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_1Correctable.Size()

	if fldName == "Rxbuf_2Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_2Correctable.Size()

	if fldName == "Rxbuf_3Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_3Correctable.Size()

	return offset
}

// SetRxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc4intceccMetrics) SetRxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Uncorrectable"))
	return nil
}

// SetRxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc4intceccMetrics) SetRxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Uncorrectable"))
	return nil
}

// SetRxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc4intceccMetrics) SetRxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Uncorrectable"))
	return nil
}

// SetRxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc4intceccMetrics) SetRxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Uncorrectable"))
	return nil
}

// SetRxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportc4intceccMetrics) SetRxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Correctable"))
	return nil
}

// SetRxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportc4intceccMetrics) SetRxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Correctable"))
	return nil
}

// SetRxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportc4intceccMetrics) SetRxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Correctable"))
	return nil
}

// SetRxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportc4intceccMetrics) SetRxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Correctable"))
	return nil
}

// Ppppportc4intceccMetricsIterator is the iterator object
type Ppppportc4intceccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc4intceccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc4intceccMetricsIterator) Next() *Ppppportc4intceccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc4intceccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc4intceccMetricsIterator) Find(key uint64) (*Ppppportc4intceccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc4intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc4intceccMetricsIterator) Create(key uint64) (*Ppppportc4intceccMetrics, error) {
	tmtr := &Ppppportc4intceccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc4intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc4intceccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc4intceccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc4intceccMetricsIterator returns an iterator
func NewPpppportc4intceccMetricsIterator() (*Ppppportc4intceccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc4intceccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc4intceccMetricsIterator{iter: iter}, nil
}

type Ppppportc5intcmacMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	TxbfrOverflow metrics.Counter

	RxtlpErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc5intcmacMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc5intcmacMetrics) Size() int {
	sz := 0

	sz += mtr.TxbfrOverflow.Size()

	sz += mtr.RxtlpErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc5intcmacMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.TxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.TxbfrOverflow.Size()

	mtr.RxtlpErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RxtlpErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc5intcmacMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "TxbfrOverflow" {
		return offset
	}
	offset += mtr.TxbfrOverflow.Size()

	if fldName == "RxtlpErr" {
		return offset
	}
	offset += mtr.RxtlpErr.Size()

	return offset
}

// SetTxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportc5intcmacMetrics) SetTxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxbfrOverflow"))
	return nil
}

// SetRxtlpErr sets cunter in shared memory
func (mtr *Ppppportc5intcmacMetrics) SetRxtlpErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxtlpErr"))
	return nil
}

// Ppppportc5intcmacMetricsIterator is the iterator object
type Ppppportc5intcmacMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc5intcmacMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc5intcmacMetricsIterator) Next() *Ppppportc5intcmacMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc5intcmacMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc5intcmacMetricsIterator) Find(key uint64) (*Ppppportc5intcmacMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc5intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc5intcmacMetricsIterator) Create(key uint64) (*Ppppportc5intcmacMetrics, error) {
	tmtr := &Ppppportc5intcmacMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc5intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc5intcmacMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc5intcmacMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc5intcmacMetricsIterator returns an iterator
func NewPpppportc5intcmacMetricsIterator() (*Ppppportc5intcmacMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc5intcmacMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc5intcmacMetricsIterator{iter: iter}, nil
}

type Ppppportc5intceccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Rxbuf_0Uncorrectable metrics.Counter

	Rxbuf_1Uncorrectable metrics.Counter

	Rxbuf_2Uncorrectable metrics.Counter

	Rxbuf_3Uncorrectable metrics.Counter

	Rxbuf_0Correctable metrics.Counter

	Rxbuf_1Correctable metrics.Counter

	Rxbuf_2Correctable metrics.Counter

	Rxbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc5intceccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc5intceccMetrics) Size() int {
	sz := 0

	sz += mtr.Rxbuf_0Uncorrectable.Size()

	sz += mtr.Rxbuf_1Uncorrectable.Size()

	sz += mtr.Rxbuf_2Uncorrectable.Size()

	sz += mtr.Rxbuf_3Uncorrectable.Size()

	sz += mtr.Rxbuf_0Correctable.Size()

	sz += mtr.Rxbuf_1Correctable.Size()

	sz += mtr.Rxbuf_2Correctable.Size()

	sz += mtr.Rxbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc5intceccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Rxbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	mtr.Rxbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	mtr.Rxbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	mtr.Rxbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	mtr.Rxbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Correctable.Size()

	mtr.Rxbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Correctable.Size()

	mtr.Rxbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Correctable.Size()

	mtr.Rxbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc5intceccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Rxbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	if fldName == "Rxbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	if fldName == "Rxbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	if fldName == "Rxbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	if fldName == "Rxbuf_0Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_0Correctable.Size()

	if fldName == "Rxbuf_1Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_1Correctable.Size()

	if fldName == "Rxbuf_2Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_2Correctable.Size()

	if fldName == "Rxbuf_3Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_3Correctable.Size()

	return offset
}

// SetRxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc5intceccMetrics) SetRxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Uncorrectable"))
	return nil
}

// SetRxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc5intceccMetrics) SetRxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Uncorrectable"))
	return nil
}

// SetRxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc5intceccMetrics) SetRxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Uncorrectable"))
	return nil
}

// SetRxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc5intceccMetrics) SetRxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Uncorrectable"))
	return nil
}

// SetRxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportc5intceccMetrics) SetRxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Correctable"))
	return nil
}

// SetRxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportc5intceccMetrics) SetRxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Correctable"))
	return nil
}

// SetRxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportc5intceccMetrics) SetRxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Correctable"))
	return nil
}

// SetRxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportc5intceccMetrics) SetRxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Correctable"))
	return nil
}

// Ppppportc5intceccMetricsIterator is the iterator object
type Ppppportc5intceccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc5intceccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc5intceccMetricsIterator) Next() *Ppppportc5intceccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc5intceccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc5intceccMetricsIterator) Find(key uint64) (*Ppppportc5intceccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc5intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc5intceccMetricsIterator) Create(key uint64) (*Ppppportc5intceccMetrics, error) {
	tmtr := &Ppppportc5intceccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc5intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc5intceccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc5intceccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc5intceccMetricsIterator returns an iterator
func NewPpppportc5intceccMetricsIterator() (*Ppppportc5intceccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc5intceccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc5intceccMetricsIterator{iter: iter}, nil
}

type Ppppportc6intcmacMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	TxbfrOverflow metrics.Counter

	RxtlpErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc6intcmacMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc6intcmacMetrics) Size() int {
	sz := 0

	sz += mtr.TxbfrOverflow.Size()

	sz += mtr.RxtlpErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc6intcmacMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.TxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.TxbfrOverflow.Size()

	mtr.RxtlpErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RxtlpErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc6intcmacMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "TxbfrOverflow" {
		return offset
	}
	offset += mtr.TxbfrOverflow.Size()

	if fldName == "RxtlpErr" {
		return offset
	}
	offset += mtr.RxtlpErr.Size()

	return offset
}

// SetTxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportc6intcmacMetrics) SetTxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxbfrOverflow"))
	return nil
}

// SetRxtlpErr sets cunter in shared memory
func (mtr *Ppppportc6intcmacMetrics) SetRxtlpErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxtlpErr"))
	return nil
}

// Ppppportc6intcmacMetricsIterator is the iterator object
type Ppppportc6intcmacMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc6intcmacMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc6intcmacMetricsIterator) Next() *Ppppportc6intcmacMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc6intcmacMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc6intcmacMetricsIterator) Find(key uint64) (*Ppppportc6intcmacMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc6intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc6intcmacMetricsIterator) Create(key uint64) (*Ppppportc6intcmacMetrics, error) {
	tmtr := &Ppppportc6intcmacMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc6intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc6intcmacMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc6intcmacMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc6intcmacMetricsIterator returns an iterator
func NewPpppportc6intcmacMetricsIterator() (*Ppppportc6intcmacMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc6intcmacMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc6intcmacMetricsIterator{iter: iter}, nil
}

type Ppppportc6intceccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Rxbuf_0Uncorrectable metrics.Counter

	Rxbuf_1Uncorrectable metrics.Counter

	Rxbuf_2Uncorrectable metrics.Counter

	Rxbuf_3Uncorrectable metrics.Counter

	Rxbuf_0Correctable metrics.Counter

	Rxbuf_1Correctable metrics.Counter

	Rxbuf_2Correctable metrics.Counter

	Rxbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc6intceccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc6intceccMetrics) Size() int {
	sz := 0

	sz += mtr.Rxbuf_0Uncorrectable.Size()

	sz += mtr.Rxbuf_1Uncorrectable.Size()

	sz += mtr.Rxbuf_2Uncorrectable.Size()

	sz += mtr.Rxbuf_3Uncorrectable.Size()

	sz += mtr.Rxbuf_0Correctable.Size()

	sz += mtr.Rxbuf_1Correctable.Size()

	sz += mtr.Rxbuf_2Correctable.Size()

	sz += mtr.Rxbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc6intceccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Rxbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	mtr.Rxbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	mtr.Rxbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	mtr.Rxbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	mtr.Rxbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Correctable.Size()

	mtr.Rxbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Correctable.Size()

	mtr.Rxbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Correctable.Size()

	mtr.Rxbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc6intceccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Rxbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	if fldName == "Rxbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	if fldName == "Rxbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	if fldName == "Rxbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	if fldName == "Rxbuf_0Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_0Correctable.Size()

	if fldName == "Rxbuf_1Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_1Correctable.Size()

	if fldName == "Rxbuf_2Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_2Correctable.Size()

	if fldName == "Rxbuf_3Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_3Correctable.Size()

	return offset
}

// SetRxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc6intceccMetrics) SetRxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Uncorrectable"))
	return nil
}

// SetRxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc6intceccMetrics) SetRxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Uncorrectable"))
	return nil
}

// SetRxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc6intceccMetrics) SetRxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Uncorrectable"))
	return nil
}

// SetRxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc6intceccMetrics) SetRxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Uncorrectable"))
	return nil
}

// SetRxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportc6intceccMetrics) SetRxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Correctable"))
	return nil
}

// SetRxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportc6intceccMetrics) SetRxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Correctable"))
	return nil
}

// SetRxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportc6intceccMetrics) SetRxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Correctable"))
	return nil
}

// SetRxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportc6intceccMetrics) SetRxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Correctable"))
	return nil
}

// Ppppportc6intceccMetricsIterator is the iterator object
type Ppppportc6intceccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc6intceccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc6intceccMetricsIterator) Next() *Ppppportc6intceccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc6intceccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc6intceccMetricsIterator) Find(key uint64) (*Ppppportc6intceccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc6intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc6intceccMetricsIterator) Create(key uint64) (*Ppppportc6intceccMetrics, error) {
	tmtr := &Ppppportc6intceccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc6intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc6intceccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc6intceccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc6intceccMetricsIterator returns an iterator
func NewPpppportc6intceccMetricsIterator() (*Ppppportc6intceccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc6intceccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc6intceccMetricsIterator{iter: iter}, nil
}

type Ppppportc7intcmacMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	TxbfrOverflow metrics.Counter

	RxtlpErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc7intcmacMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc7intcmacMetrics) Size() int {
	sz := 0

	sz += mtr.TxbfrOverflow.Size()

	sz += mtr.RxtlpErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc7intcmacMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.TxbfrOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.TxbfrOverflow.Size()

	mtr.RxtlpErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RxtlpErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc7intcmacMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "TxbfrOverflow" {
		return offset
	}
	offset += mtr.TxbfrOverflow.Size()

	if fldName == "RxtlpErr" {
		return offset
	}
	offset += mtr.RxtlpErr.Size()

	return offset
}

// SetTxbfrOverflow sets cunter in shared memory
func (mtr *Ppppportc7intcmacMetrics) SetTxbfrOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxbfrOverflow"))
	return nil
}

// SetRxtlpErr sets cunter in shared memory
func (mtr *Ppppportc7intcmacMetrics) SetRxtlpErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxtlpErr"))
	return nil
}

// Ppppportc7intcmacMetricsIterator is the iterator object
type Ppppportc7intcmacMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc7intcmacMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc7intcmacMetricsIterator) Next() *Ppppportc7intcmacMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc7intcmacMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc7intcmacMetricsIterator) Find(key uint64) (*Ppppportc7intcmacMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc7intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc7intcmacMetricsIterator) Create(key uint64) (*Ppppportc7intcmacMetrics, error) {
	tmtr := &Ppppportc7intcmacMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc7intcmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc7intcmacMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc7intcmacMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc7intcmacMetricsIterator returns an iterator
func NewPpppportc7intcmacMetricsIterator() (*Ppppportc7intcmacMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc7intcmacMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc7intcmacMetricsIterator{iter: iter}, nil
}

type Ppppportc7intceccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Rxbuf_0Uncorrectable metrics.Counter

	Rxbuf_1Uncorrectable metrics.Counter

	Rxbuf_2Uncorrectable metrics.Counter

	Rxbuf_3Uncorrectable metrics.Counter

	Rxbuf_0Correctable metrics.Counter

	Rxbuf_1Correctable metrics.Counter

	Rxbuf_2Correctable metrics.Counter

	Rxbuf_3Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ppppportc7intceccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ppppportc7intceccMetrics) Size() int {
	sz := 0

	sz += mtr.Rxbuf_0Uncorrectable.Size()

	sz += mtr.Rxbuf_1Uncorrectable.Size()

	sz += mtr.Rxbuf_2Uncorrectable.Size()

	sz += mtr.Rxbuf_3Uncorrectable.Size()

	sz += mtr.Rxbuf_0Correctable.Size()

	sz += mtr.Rxbuf_1Correctable.Size()

	sz += mtr.Rxbuf_2Correctable.Size()

	sz += mtr.Rxbuf_3Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ppppportc7intceccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Rxbuf_0Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	mtr.Rxbuf_1Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	mtr.Rxbuf_2Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	mtr.Rxbuf_3Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	mtr.Rxbuf_0Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_0Correctable.Size()

	mtr.Rxbuf_1Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_1Correctable.Size()

	mtr.Rxbuf_2Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_2Correctable.Size()

	mtr.Rxbuf_3Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Rxbuf_3Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ppppportc7intceccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Rxbuf_0Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_0Uncorrectable.Size()

	if fldName == "Rxbuf_1Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_1Uncorrectable.Size()

	if fldName == "Rxbuf_2Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_2Uncorrectable.Size()

	if fldName == "Rxbuf_3Uncorrectable" {
		return offset
	}
	offset += mtr.Rxbuf_3Uncorrectable.Size()

	if fldName == "Rxbuf_0Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_0Correctable.Size()

	if fldName == "Rxbuf_1Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_1Correctable.Size()

	if fldName == "Rxbuf_2Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_2Correctable.Size()

	if fldName == "Rxbuf_3Correctable" {
		return offset
	}
	offset += mtr.Rxbuf_3Correctable.Size()

	return offset
}

// SetRxbuf_0Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc7intceccMetrics) SetRxbuf_0Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Uncorrectable"))
	return nil
}

// SetRxbuf_1Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc7intceccMetrics) SetRxbuf_1Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Uncorrectable"))
	return nil
}

// SetRxbuf_2Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc7intceccMetrics) SetRxbuf_2Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Uncorrectable"))
	return nil
}

// SetRxbuf_3Uncorrectable sets cunter in shared memory
func (mtr *Ppppportc7intceccMetrics) SetRxbuf_3Uncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Uncorrectable"))
	return nil
}

// SetRxbuf_0Correctable sets cunter in shared memory
func (mtr *Ppppportc7intceccMetrics) SetRxbuf_0Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_0Correctable"))
	return nil
}

// SetRxbuf_1Correctable sets cunter in shared memory
func (mtr *Ppppportc7intceccMetrics) SetRxbuf_1Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_1Correctable"))
	return nil
}

// SetRxbuf_2Correctable sets cunter in shared memory
func (mtr *Ppppportc7intceccMetrics) SetRxbuf_2Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_2Correctable"))
	return nil
}

// SetRxbuf_3Correctable sets cunter in shared memory
func (mtr *Ppppportc7intceccMetrics) SetRxbuf_3Correctable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rxbuf_3Correctable"))
	return nil
}

// Ppppportc7intceccMetricsIterator is the iterator object
type Ppppportc7intceccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ppppportc7intceccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ppppportc7intceccMetricsIterator) Next() *Ppppportc7intceccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ppppportc7intceccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ppppportc7intceccMetricsIterator) Find(key uint64) (*Ppppportc7intceccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ppppportc7intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ppppportc7intceccMetricsIterator) Create(key uint64) (*Ppppportc7intceccMetrics, error) {
	tmtr := &Ppppportc7intceccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ppppportc7intceccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ppppportc7intceccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ppppportc7intceccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPpppportc7intceccMetricsIterator returns an iterator
func NewPpppportc7intceccMetricsIterator() (*Ppppportc7intceccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ppppportc7intceccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ppppportc7intceccMetricsIterator{iter: iter}, nil
}

type PtptpspinterrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PbPbusErr metrics.Counter

	PrPbusErr metrics.Counter

	PrResubPbusErr metrics.Counter

	PspCsrReadAccessErr metrics.Counter

	PspCsrWriteAccessErr metrics.Counter

	PspPhvLargerThanTwelveFlitsErr metrics.Counter

	PspPhvSopOffsetErr metrics.Counter

	PspPhvEopOffsetErr metrics.Counter

	MaxRecirCountErr metrics.Counter

	PbPbusDummyErr metrics.Counter

	PbPbusNopktErr metrics.Counter

	PbPbusMinErr metrics.Counter

	LbPhvAlmostFullTimeoutErr metrics.Counter

	LbPktAlmostFullTimeoutErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PtptpspinterrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PtptpspinterrMetrics) Size() int {
	sz := 0

	sz += mtr.PbPbusErr.Size()

	sz += mtr.PrPbusErr.Size()

	sz += mtr.PrResubPbusErr.Size()

	sz += mtr.PspCsrReadAccessErr.Size()

	sz += mtr.PspCsrWriteAccessErr.Size()

	sz += mtr.PspPhvLargerThanTwelveFlitsErr.Size()

	sz += mtr.PspPhvSopOffsetErr.Size()

	sz += mtr.PspPhvEopOffsetErr.Size()

	sz += mtr.MaxRecirCountErr.Size()

	sz += mtr.PbPbusDummyErr.Size()

	sz += mtr.PbPbusNopktErr.Size()

	sz += mtr.PbPbusMinErr.Size()

	sz += mtr.LbPhvAlmostFullTimeoutErr.Size()

	sz += mtr.LbPktAlmostFullTimeoutErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PtptpspinterrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PbPbusErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusErr.Size()

	mtr.PrPbusErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrPbusErr.Size()

	mtr.PrResubPbusErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrResubPbusErr.Size()

	mtr.PspCsrReadAccessErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PspCsrReadAccessErr.Size()

	mtr.PspCsrWriteAccessErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PspCsrWriteAccessErr.Size()

	mtr.PspPhvLargerThanTwelveFlitsErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PspPhvLargerThanTwelveFlitsErr.Size()

	mtr.PspPhvSopOffsetErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PspPhvSopOffsetErr.Size()

	mtr.PspPhvEopOffsetErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PspPhvEopOffsetErr.Size()

	mtr.MaxRecirCountErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxRecirCountErr.Size()

	mtr.PbPbusDummyErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusDummyErr.Size()

	mtr.PbPbusNopktErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusNopktErr.Size()

	mtr.PbPbusMinErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusMinErr.Size()

	mtr.LbPhvAlmostFullTimeoutErr = mtr.metrics.GetCounter(offset)
	offset += mtr.LbPhvAlmostFullTimeoutErr.Size()

	mtr.LbPktAlmostFullTimeoutErr = mtr.metrics.GetCounter(offset)
	offset += mtr.LbPktAlmostFullTimeoutErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PtptpspinterrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PbPbusErr" {
		return offset
	}
	offset += mtr.PbPbusErr.Size()

	if fldName == "PrPbusErr" {
		return offset
	}
	offset += mtr.PrPbusErr.Size()

	if fldName == "PrResubPbusErr" {
		return offset
	}
	offset += mtr.PrResubPbusErr.Size()

	if fldName == "PspCsrReadAccessErr" {
		return offset
	}
	offset += mtr.PspCsrReadAccessErr.Size()

	if fldName == "PspCsrWriteAccessErr" {
		return offset
	}
	offset += mtr.PspCsrWriteAccessErr.Size()

	if fldName == "PspPhvLargerThanTwelveFlitsErr" {
		return offset
	}
	offset += mtr.PspPhvLargerThanTwelveFlitsErr.Size()

	if fldName == "PspPhvSopOffsetErr" {
		return offset
	}
	offset += mtr.PspPhvSopOffsetErr.Size()

	if fldName == "PspPhvEopOffsetErr" {
		return offset
	}
	offset += mtr.PspPhvEopOffsetErr.Size()

	if fldName == "MaxRecirCountErr" {
		return offset
	}
	offset += mtr.MaxRecirCountErr.Size()

	if fldName == "PbPbusDummyErr" {
		return offset
	}
	offset += mtr.PbPbusDummyErr.Size()

	if fldName == "PbPbusNopktErr" {
		return offset
	}
	offset += mtr.PbPbusNopktErr.Size()

	if fldName == "PbPbusMinErr" {
		return offset
	}
	offset += mtr.PbPbusMinErr.Size()

	if fldName == "LbPhvAlmostFullTimeoutErr" {
		return offset
	}
	offset += mtr.LbPhvAlmostFullTimeoutErr.Size()

	if fldName == "LbPktAlmostFullTimeoutErr" {
		return offset
	}
	offset += mtr.LbPktAlmostFullTimeoutErr.Size()

	return offset
}

// SetPbPbusErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetPbPbusErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusErr"))
	return nil
}

// SetPrPbusErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetPrPbusErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrPbusErr"))
	return nil
}

// SetPrResubPbusErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetPrResubPbusErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrResubPbusErr"))
	return nil
}

// SetPspCsrReadAccessErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetPspCsrReadAccessErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PspCsrReadAccessErr"))
	return nil
}

// SetPspCsrWriteAccessErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetPspCsrWriteAccessErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PspCsrWriteAccessErr"))
	return nil
}

// SetPspPhvLargerThanTwelveFlitsErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetPspPhvLargerThanTwelveFlitsErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PspPhvLargerThanTwelveFlitsErr"))
	return nil
}

// SetPspPhvSopOffsetErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetPspPhvSopOffsetErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PspPhvSopOffsetErr"))
	return nil
}

// SetPspPhvEopOffsetErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetPspPhvEopOffsetErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PspPhvEopOffsetErr"))
	return nil
}

// SetMaxRecirCountErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetMaxRecirCountErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxRecirCountErr"))
	return nil
}

// SetPbPbusDummyErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetPbPbusDummyErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusDummyErr"))
	return nil
}

// SetPbPbusNopktErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetPbPbusNopktErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusNopktErr"))
	return nil
}

// SetPbPbusMinErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetPbPbusMinErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusMinErr"))
	return nil
}

// SetLbPhvAlmostFullTimeoutErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetLbPhvAlmostFullTimeoutErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LbPhvAlmostFullTimeoutErr"))
	return nil
}

// SetLbPktAlmostFullTimeoutErr sets cunter in shared memory
func (mtr *PtptpspinterrMetrics) SetLbPktAlmostFullTimeoutErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LbPktAlmostFullTimeoutErr"))
	return nil
}

// PtptpspinterrMetricsIterator is the iterator object
type PtptpspinterrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PtptpspinterrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PtptpspinterrMetricsIterator) Next() *PtptpspinterrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PtptpspinterrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PtptpspinterrMetricsIterator) Find(key uint64) (*PtptpspinterrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PtptpspinterrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PtptpspinterrMetricsIterator) Create(key uint64) (*PtptpspinterrMetrics, error) {
	tmtr := &PtptpspinterrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PtptpspinterrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PtptpspinterrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PtptpspinterrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPtptpspinterrMetricsIterator returns an iterator
func NewPtptpspinterrMetricsIterator() (*PtptpspinterrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PtptpspinterrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PtptpspinterrMetricsIterator{iter: iter}, nil
}

type PtptpspintfatalMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PbPbusSopErr metrics.Counter

	PbPbusEopErr metrics.Counter

	MaSopErr metrics.Counter

	MaEopErr metrics.Counter

	PrPbusSopErr metrics.Counter

	PrPbusEopErr metrics.Counter

	PrResubPbusSopErr metrics.Counter

	PrResubPbusEopErr metrics.Counter

	PrResubSopErr metrics.Counter

	PrResubEopErr metrics.Counter

	PbPbusFsm0NoDataErr metrics.Counter

	PbPbusFsm1NoDataErr metrics.Counter

	PbPbusFsm2NoDataErr metrics.Counter

	PbPbusFsm3NoDataErr metrics.Counter

	PbPbusFsm4NoDataErr metrics.Counter

	PbPbusFsm5PktTooSmallErr metrics.Counter

	LifIndTableRspErr metrics.Counter

	FlitCntOflowErr metrics.Counter

	LbPhvSopErr metrics.Counter

	DcPhvSopErr metrics.Counter

	LbPktSopErr metrics.Counter

	DcPktSopErr metrics.Counter

	SwPhvJabberErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PtptpspintfatalMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PtptpspintfatalMetrics) Size() int {
	sz := 0

	sz += mtr.PbPbusSopErr.Size()

	sz += mtr.PbPbusEopErr.Size()

	sz += mtr.MaSopErr.Size()

	sz += mtr.MaEopErr.Size()

	sz += mtr.PrPbusSopErr.Size()

	sz += mtr.PrPbusEopErr.Size()

	sz += mtr.PrResubPbusSopErr.Size()

	sz += mtr.PrResubPbusEopErr.Size()

	sz += mtr.PrResubSopErr.Size()

	sz += mtr.PrResubEopErr.Size()

	sz += mtr.PbPbusFsm0NoDataErr.Size()

	sz += mtr.PbPbusFsm1NoDataErr.Size()

	sz += mtr.PbPbusFsm2NoDataErr.Size()

	sz += mtr.PbPbusFsm3NoDataErr.Size()

	sz += mtr.PbPbusFsm4NoDataErr.Size()

	sz += mtr.PbPbusFsm5PktTooSmallErr.Size()

	sz += mtr.LifIndTableRspErr.Size()

	sz += mtr.FlitCntOflowErr.Size()

	sz += mtr.LbPhvSopErr.Size()

	sz += mtr.DcPhvSopErr.Size()

	sz += mtr.LbPktSopErr.Size()

	sz += mtr.DcPktSopErr.Size()

	sz += mtr.SwPhvJabberErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PtptpspintfatalMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PbPbusSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusSopErr.Size()

	mtr.PbPbusEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusEopErr.Size()

	mtr.MaSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaSopErr.Size()

	mtr.MaEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaEopErr.Size()

	mtr.PrPbusSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrPbusSopErr.Size()

	mtr.PrPbusEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrPbusEopErr.Size()

	mtr.PrResubPbusSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrResubPbusSopErr.Size()

	mtr.PrResubPbusEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrResubPbusEopErr.Size()

	mtr.PrResubSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrResubSopErr.Size()

	mtr.PrResubEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PrResubEopErr.Size()

	mtr.PbPbusFsm0NoDataErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusFsm0NoDataErr.Size()

	mtr.PbPbusFsm1NoDataErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusFsm1NoDataErr.Size()

	mtr.PbPbusFsm2NoDataErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusFsm2NoDataErr.Size()

	mtr.PbPbusFsm3NoDataErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusFsm3NoDataErr.Size()

	mtr.PbPbusFsm4NoDataErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusFsm4NoDataErr.Size()

	mtr.PbPbusFsm5PktTooSmallErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbPbusFsm5PktTooSmallErr.Size()

	mtr.LifIndTableRspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.LifIndTableRspErr.Size()

	mtr.FlitCntOflowErr = mtr.metrics.GetCounter(offset)
	offset += mtr.FlitCntOflowErr.Size()

	mtr.LbPhvSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.LbPhvSopErr.Size()

	mtr.DcPhvSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.DcPhvSopErr.Size()

	mtr.LbPktSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.LbPktSopErr.Size()

	mtr.DcPktSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.DcPktSopErr.Size()

	mtr.SwPhvJabberErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SwPhvJabberErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PtptpspintfatalMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PbPbusSopErr" {
		return offset
	}
	offset += mtr.PbPbusSopErr.Size()

	if fldName == "PbPbusEopErr" {
		return offset
	}
	offset += mtr.PbPbusEopErr.Size()

	if fldName == "MaSopErr" {
		return offset
	}
	offset += mtr.MaSopErr.Size()

	if fldName == "MaEopErr" {
		return offset
	}
	offset += mtr.MaEopErr.Size()

	if fldName == "PrPbusSopErr" {
		return offset
	}
	offset += mtr.PrPbusSopErr.Size()

	if fldName == "PrPbusEopErr" {
		return offset
	}
	offset += mtr.PrPbusEopErr.Size()

	if fldName == "PrResubPbusSopErr" {
		return offset
	}
	offset += mtr.PrResubPbusSopErr.Size()

	if fldName == "PrResubPbusEopErr" {
		return offset
	}
	offset += mtr.PrResubPbusEopErr.Size()

	if fldName == "PrResubSopErr" {
		return offset
	}
	offset += mtr.PrResubSopErr.Size()

	if fldName == "PrResubEopErr" {
		return offset
	}
	offset += mtr.PrResubEopErr.Size()

	if fldName == "PbPbusFsm0NoDataErr" {
		return offset
	}
	offset += mtr.PbPbusFsm0NoDataErr.Size()

	if fldName == "PbPbusFsm1NoDataErr" {
		return offset
	}
	offset += mtr.PbPbusFsm1NoDataErr.Size()

	if fldName == "PbPbusFsm2NoDataErr" {
		return offset
	}
	offset += mtr.PbPbusFsm2NoDataErr.Size()

	if fldName == "PbPbusFsm3NoDataErr" {
		return offset
	}
	offset += mtr.PbPbusFsm3NoDataErr.Size()

	if fldName == "PbPbusFsm4NoDataErr" {
		return offset
	}
	offset += mtr.PbPbusFsm4NoDataErr.Size()

	if fldName == "PbPbusFsm5PktTooSmallErr" {
		return offset
	}
	offset += mtr.PbPbusFsm5PktTooSmallErr.Size()

	if fldName == "LifIndTableRspErr" {
		return offset
	}
	offset += mtr.LifIndTableRspErr.Size()

	if fldName == "FlitCntOflowErr" {
		return offset
	}
	offset += mtr.FlitCntOflowErr.Size()

	if fldName == "LbPhvSopErr" {
		return offset
	}
	offset += mtr.LbPhvSopErr.Size()

	if fldName == "DcPhvSopErr" {
		return offset
	}
	offset += mtr.DcPhvSopErr.Size()

	if fldName == "LbPktSopErr" {
		return offset
	}
	offset += mtr.LbPktSopErr.Size()

	if fldName == "DcPktSopErr" {
		return offset
	}
	offset += mtr.DcPktSopErr.Size()

	if fldName == "SwPhvJabberErr" {
		return offset
	}
	offset += mtr.SwPhvJabberErr.Size()

	return offset
}

// SetPbPbusSopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPbPbusSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusSopErr"))
	return nil
}

// SetPbPbusEopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPbPbusEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusEopErr"))
	return nil
}

// SetMaSopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetMaSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaSopErr"))
	return nil
}

// SetMaEopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetMaEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaEopErr"))
	return nil
}

// SetPrPbusSopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPrPbusSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrPbusSopErr"))
	return nil
}

// SetPrPbusEopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPrPbusEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrPbusEopErr"))
	return nil
}

// SetPrResubPbusSopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPrResubPbusSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrResubPbusSopErr"))
	return nil
}

// SetPrResubPbusEopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPrResubPbusEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrResubPbusEopErr"))
	return nil
}

// SetPrResubSopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPrResubSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrResubSopErr"))
	return nil
}

// SetPrResubEopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPrResubEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PrResubEopErr"))
	return nil
}

// SetPbPbusFsm0NoDataErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPbPbusFsm0NoDataErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusFsm0NoDataErr"))
	return nil
}

// SetPbPbusFsm1NoDataErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPbPbusFsm1NoDataErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusFsm1NoDataErr"))
	return nil
}

// SetPbPbusFsm2NoDataErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPbPbusFsm2NoDataErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusFsm2NoDataErr"))
	return nil
}

// SetPbPbusFsm3NoDataErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPbPbusFsm3NoDataErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusFsm3NoDataErr"))
	return nil
}

// SetPbPbusFsm4NoDataErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPbPbusFsm4NoDataErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusFsm4NoDataErr"))
	return nil
}

// SetPbPbusFsm5PktTooSmallErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetPbPbusFsm5PktTooSmallErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbPbusFsm5PktTooSmallErr"))
	return nil
}

// SetLifIndTableRspErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetLifIndTableRspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LifIndTableRspErr"))
	return nil
}

// SetFlitCntOflowErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetFlitCntOflowErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FlitCntOflowErr"))
	return nil
}

// SetLbPhvSopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetLbPhvSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LbPhvSopErr"))
	return nil
}

// SetDcPhvSopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetDcPhvSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DcPhvSopErr"))
	return nil
}

// SetLbPktSopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetLbPktSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LbPktSopErr"))
	return nil
}

// SetDcPktSopErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetDcPktSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DcPktSopErr"))
	return nil
}

// SetSwPhvJabberErr sets cunter in shared memory
func (mtr *PtptpspintfatalMetrics) SetSwPhvJabberErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SwPhvJabberErr"))
	return nil
}

// PtptpspintfatalMetricsIterator is the iterator object
type PtptpspintfatalMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PtptpspintfatalMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PtptpspintfatalMetricsIterator) Next() *PtptpspintfatalMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PtptpspintfatalMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PtptpspintfatalMetricsIterator) Find(key uint64) (*PtptpspintfatalMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PtptpspintfatalMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PtptpspintfatalMetricsIterator) Create(key uint64) (*PtptpspintfatalMetrics, error) {
	tmtr := &PtptpspintfatalMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PtptpspintfatalMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PtptpspintfatalMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PtptpspintfatalMetricsIterator) Free() {
	it.iter.Free()
}

// NewPtptpspintfatalMetricsIterator returns an iterator
func NewPtptpspintfatalMetricsIterator() (*PtptpspintfatalMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PtptpspintfatalMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PtptpspintfatalMetricsIterator{iter: iter}, nil
}

type PtptpspintlifqstatemapMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	EccUncorrectable metrics.Counter

	EccCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PtptpspintlifqstatemapMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PtptpspintlifqstatemapMetrics) Size() int {
	sz := 0

	sz += mtr.EccUncorrectable.Size()

	sz += mtr.EccCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PtptpspintlifqstatemapMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.EccUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccUncorrectable.Size()

	mtr.EccCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PtptpspintlifqstatemapMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "EccUncorrectable" {
		return offset
	}
	offset += mtr.EccUncorrectable.Size()

	if fldName == "EccCorrectable" {
		return offset
	}
	offset += mtr.EccCorrectable.Size()

	return offset
}

// SetEccUncorrectable sets cunter in shared memory
func (mtr *PtptpspintlifqstatemapMetrics) SetEccUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccUncorrectable"))
	return nil
}

// SetEccCorrectable sets cunter in shared memory
func (mtr *PtptpspintlifqstatemapMetrics) SetEccCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccCorrectable"))
	return nil
}

// PtptpspintlifqstatemapMetricsIterator is the iterator object
type PtptpspintlifqstatemapMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PtptpspintlifqstatemapMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PtptpspintlifqstatemapMetricsIterator) Next() *PtptpspintlifqstatemapMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PtptpspintlifqstatemapMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PtptpspintlifqstatemapMetricsIterator) Find(key uint64) (*PtptpspintlifqstatemapMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PtptpspintlifqstatemapMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PtptpspintlifqstatemapMetricsIterator) Create(key uint64) (*PtptpspintlifqstatemapMetrics, error) {
	tmtr := &PtptpspintlifqstatemapMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PtptpspintlifqstatemapMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PtptpspintlifqstatemapMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PtptpspintlifqstatemapMetricsIterator) Free() {
	it.iter.Free()
}

// NewPtptpspintlifqstatemapMetricsIterator returns an iterator
func NewPtptpspintlifqstatemapMetricsIterator() (*PtptpspintlifqstatemapMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PtptpspintlifqstatemapMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PtptpspintlifqstatemapMetricsIterator{iter: iter}, nil
}

type PtptpspintswphvmemMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	EccUncorrectable metrics.Counter

	EccCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PtptpspintswphvmemMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PtptpspintswphvmemMetrics) Size() int {
	sz := 0

	sz += mtr.EccUncorrectable.Size()

	sz += mtr.EccCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PtptpspintswphvmemMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.EccUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccUncorrectable.Size()

	mtr.EccCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PtptpspintswphvmemMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "EccUncorrectable" {
		return offset
	}
	offset += mtr.EccUncorrectable.Size()

	if fldName == "EccCorrectable" {
		return offset
	}
	offset += mtr.EccCorrectable.Size()

	return offset
}

// SetEccUncorrectable sets cunter in shared memory
func (mtr *PtptpspintswphvmemMetrics) SetEccUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccUncorrectable"))
	return nil
}

// SetEccCorrectable sets cunter in shared memory
func (mtr *PtptpspintswphvmemMetrics) SetEccCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccCorrectable"))
	return nil
}

// PtptpspintswphvmemMetricsIterator is the iterator object
type PtptpspintswphvmemMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PtptpspintswphvmemMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PtptpspintswphvmemMetricsIterator) Next() *PtptpspintswphvmemMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PtptpspintswphvmemMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PtptpspintswphvmemMetricsIterator) Find(key uint64) (*PtptpspintswphvmemMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PtptpspintswphvmemMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PtptpspintswphvmemMetricsIterator) Create(key uint64) (*PtptpspintswphvmemMetrics, error) {
	tmtr := &PtptpspintswphvmemMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PtptpspintswphvmemMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PtptpspintswphvmemMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PtptpspintswphvmemMetricsIterator) Free() {
	it.iter.Free()
}

// NewPtptpspintswphvmemMetricsIterator returns an iterator
func NewPtptpspintswphvmemMetricsIterator() (*PtptpspintswphvmemMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PtptpspintswphvmemMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PtptpspintswphvmemMetricsIterator{iter: iter}, nil
}

type PtptptdinteccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RdataMemUncorrectable metrics.Counter

	RdataMemCorrectable metrics.Counter

	LatMemUncorrectable metrics.Counter

	LatMemCorrectable metrics.Counter

	WdataMemUncorrectable metrics.Counter

	WdataMemCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PtptptdinteccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PtptptdinteccMetrics) Size() int {
	sz := 0

	sz += mtr.RdataMemUncorrectable.Size()

	sz += mtr.RdataMemCorrectable.Size()

	sz += mtr.LatMemUncorrectable.Size()

	sz += mtr.LatMemCorrectable.Size()

	sz += mtr.WdataMemUncorrectable.Size()

	sz += mtr.WdataMemCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PtptptdinteccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RdataMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RdataMemUncorrectable.Size()

	mtr.RdataMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RdataMemCorrectable.Size()

	mtr.LatMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.LatMemUncorrectable.Size()

	mtr.LatMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.LatMemCorrectable.Size()

	mtr.WdataMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.WdataMemUncorrectable.Size()

	mtr.WdataMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.WdataMemCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PtptptdinteccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RdataMemUncorrectable" {
		return offset
	}
	offset += mtr.RdataMemUncorrectable.Size()

	if fldName == "RdataMemCorrectable" {
		return offset
	}
	offset += mtr.RdataMemCorrectable.Size()

	if fldName == "LatMemUncorrectable" {
		return offset
	}
	offset += mtr.LatMemUncorrectable.Size()

	if fldName == "LatMemCorrectable" {
		return offset
	}
	offset += mtr.LatMemCorrectable.Size()

	if fldName == "WdataMemUncorrectable" {
		return offset
	}
	offset += mtr.WdataMemUncorrectable.Size()

	if fldName == "WdataMemCorrectable" {
		return offset
	}
	offset += mtr.WdataMemCorrectable.Size()

	return offset
}

// SetRdataMemUncorrectable sets cunter in shared memory
func (mtr *PtptptdinteccMetrics) SetRdataMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdataMemUncorrectable"))
	return nil
}

// SetRdataMemCorrectable sets cunter in shared memory
func (mtr *PtptptdinteccMetrics) SetRdataMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdataMemCorrectable"))
	return nil
}

// SetLatMemUncorrectable sets cunter in shared memory
func (mtr *PtptptdinteccMetrics) SetLatMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LatMemUncorrectable"))
	return nil
}

// SetLatMemCorrectable sets cunter in shared memory
func (mtr *PtptptdinteccMetrics) SetLatMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LatMemCorrectable"))
	return nil
}

// SetWdataMemUncorrectable sets cunter in shared memory
func (mtr *PtptptdinteccMetrics) SetWdataMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WdataMemUncorrectable"))
	return nil
}

// SetWdataMemCorrectable sets cunter in shared memory
func (mtr *PtptptdinteccMetrics) SetWdataMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WdataMemCorrectable"))
	return nil
}

// PtptptdinteccMetricsIterator is the iterator object
type PtptptdinteccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PtptptdinteccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PtptptdinteccMetricsIterator) Next() *PtptptdinteccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PtptptdinteccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PtptptdinteccMetricsIterator) Find(key uint64) (*PtptptdinteccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PtptptdinteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PtptptdinteccMetricsIterator) Create(key uint64) (*PtptptdinteccMetrics, error) {
	tmtr := &PtptptdinteccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PtptptdinteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PtptptdinteccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PtptptdinteccMetricsIterator) Free() {
	it.iter.Free()
}

// NewPtptptdinteccMetricsIterator returns an iterator
func NewPtptptdinteccMetricsIterator() (*PtptptdinteccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PtptptdinteccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PtptptdinteccMetricsIterator{iter: iter}, nil
}

type PtptptdintfifoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	LatFfOvflow metrics.Counter

	WdataFfOvflow metrics.Counter

	RcvStgFfOvflow metrics.Counter

	CmdflitFfOvflow metrics.Counter

	CmdFfOvflow metrics.Counter

	PktFfOvflow metrics.Counter

	WrMemFfOvflow metrics.Counter

	DfenceFfOvflow metrics.Counter

	FfenceFfOvflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PtptptdintfifoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PtptptdintfifoMetrics) Size() int {
	sz := 0

	sz += mtr.LatFfOvflow.Size()

	sz += mtr.WdataFfOvflow.Size()

	sz += mtr.RcvStgFfOvflow.Size()

	sz += mtr.CmdflitFfOvflow.Size()

	sz += mtr.CmdFfOvflow.Size()

	sz += mtr.PktFfOvflow.Size()

	sz += mtr.WrMemFfOvflow.Size()

	sz += mtr.DfenceFfOvflow.Size()

	sz += mtr.FfenceFfOvflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PtptptdintfifoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.LatFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.LatFfOvflow.Size()

	mtr.WdataFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.WdataFfOvflow.Size()

	mtr.RcvStgFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvStgFfOvflow.Size()

	mtr.CmdflitFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CmdflitFfOvflow.Size()

	mtr.CmdFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CmdFfOvflow.Size()

	mtr.PktFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktFfOvflow.Size()

	mtr.WrMemFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.WrMemFfOvflow.Size()

	mtr.DfenceFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DfenceFfOvflow.Size()

	mtr.FfenceFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.FfenceFfOvflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PtptptdintfifoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "LatFfOvflow" {
		return offset
	}
	offset += mtr.LatFfOvflow.Size()

	if fldName == "WdataFfOvflow" {
		return offset
	}
	offset += mtr.WdataFfOvflow.Size()

	if fldName == "RcvStgFfOvflow" {
		return offset
	}
	offset += mtr.RcvStgFfOvflow.Size()

	if fldName == "CmdflitFfOvflow" {
		return offset
	}
	offset += mtr.CmdflitFfOvflow.Size()

	if fldName == "CmdFfOvflow" {
		return offset
	}
	offset += mtr.CmdFfOvflow.Size()

	if fldName == "PktFfOvflow" {
		return offset
	}
	offset += mtr.PktFfOvflow.Size()

	if fldName == "WrMemFfOvflow" {
		return offset
	}
	offset += mtr.WrMemFfOvflow.Size()

	if fldName == "DfenceFfOvflow" {
		return offset
	}
	offset += mtr.DfenceFfOvflow.Size()

	if fldName == "FfenceFfOvflow" {
		return offset
	}
	offset += mtr.FfenceFfOvflow.Size()

	return offset
}

// SetLatFfOvflow sets cunter in shared memory
func (mtr *PtptptdintfifoMetrics) SetLatFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LatFfOvflow"))
	return nil
}

// SetWdataFfOvflow sets cunter in shared memory
func (mtr *PtptptdintfifoMetrics) SetWdataFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WdataFfOvflow"))
	return nil
}

// SetRcvStgFfOvflow sets cunter in shared memory
func (mtr *PtptptdintfifoMetrics) SetRcvStgFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvStgFfOvflow"))
	return nil
}

// SetCmdflitFfOvflow sets cunter in shared memory
func (mtr *PtptptdintfifoMetrics) SetCmdflitFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CmdflitFfOvflow"))
	return nil
}

// SetCmdFfOvflow sets cunter in shared memory
func (mtr *PtptptdintfifoMetrics) SetCmdFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CmdFfOvflow"))
	return nil
}

// SetPktFfOvflow sets cunter in shared memory
func (mtr *PtptptdintfifoMetrics) SetPktFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktFfOvflow"))
	return nil
}

// SetWrMemFfOvflow sets cunter in shared memory
func (mtr *PtptptdintfifoMetrics) SetWrMemFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrMemFfOvflow"))
	return nil
}

// SetDfenceFfOvflow sets cunter in shared memory
func (mtr *PtptptdintfifoMetrics) SetDfenceFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DfenceFfOvflow"))
	return nil
}

// SetFfenceFfOvflow sets cunter in shared memory
func (mtr *PtptptdintfifoMetrics) SetFfenceFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FfenceFfOvflow"))
	return nil
}

// PtptptdintfifoMetricsIterator is the iterator object
type PtptptdintfifoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PtptptdintfifoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PtptptdintfifoMetricsIterator) Next() *PtptptdintfifoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PtptptdintfifoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PtptptdintfifoMetricsIterator) Find(key uint64) (*PtptptdintfifoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PtptptdintfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PtptptdintfifoMetricsIterator) Create(key uint64) (*PtptptdintfifoMetrics, error) {
	tmtr := &PtptptdintfifoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PtptptdintfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PtptptdintfifoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PtptptdintfifoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPtptptdintfifoMetricsIterator returns an iterator
func NewPtptptdintfifoMetricsIterator() (*PtptptdintfifoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PtptptdintfifoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PtptptdintfifoMetricsIterator{iter: iter}, nil
}
