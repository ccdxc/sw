// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"encoding/json"

	"github.com/golang/protobuf/proto"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
)

type ExampleMetrics struct {
	key uint32

	RxPkts gometrics.Counter

	TxPkts gometrics.Counter

	RxPktRate gometrics.Gauge

	TxPktRate gometrics.Gauge

	RxErrors gometrics.Counter

	TxErrors gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *ExampleMetrics) GetKey() uint32 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *ExampleMetrics) Size() int {
	sz := 0

	sz += mtr.RxPkts.Size()

	sz += mtr.TxPkts.Size()

	sz += mtr.RxPktRate.Size()

	sz += mtr.TxPktRate.Size()

	sz += mtr.RxErrors.Size()

	sz += mtr.TxErrors.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *ExampleMetrics) Unmarshal() error {
	var offset int

	val, _ := proto.DecodeVarint([]byte(mtr.metrics.GetKey()))
	mtr.key = uint32(val)

	mtr.RxPkts = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPkts.Size()

	mtr.TxPkts = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPkts.Size()

	mtr.RxPktRate = mtr.metrics.GetGauge(offset)
	offset += mtr.RxPktRate.Size()

	mtr.TxPktRate = mtr.metrics.GetGauge(offset)
	offset += mtr.TxPktRate.Size()

	mtr.RxErrors = mtr.metrics.GetCounter(offset)
	offset += mtr.RxErrors.Size()

	mtr.TxErrors = mtr.metrics.GetCounter(offset)
	offset += mtr.TxErrors.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *ExampleMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxPkts" {
		return offset
	}
	offset += mtr.RxPkts.Size()

	if fldName == "TxPkts" {
		return offset
	}
	offset += mtr.TxPkts.Size()

	if fldName == "RxPktRate" {
		return offset
	}
	offset += mtr.RxPktRate.Size()

	if fldName == "TxPktRate" {
		return offset
	}
	offset += mtr.TxPktRate.Size()

	if fldName == "RxErrors" {
		return offset
	}
	offset += mtr.RxErrors.Size()

	if fldName == "TxErrors" {
		return offset
	}
	offset += mtr.TxErrors.Size()

	return offset
}

// SetRxPkts sets cunter in shared memory
func (mtr *ExampleMetrics) SetRxPkts(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPkts"))
	return nil
}

// SetTxPkts sets cunter in shared memory
func (mtr *ExampleMetrics) SetTxPkts(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPkts"))
	return nil
}

// SetRxPktRate sets gauge in shared memory
func (mtr *ExampleMetrics) SetRxPktRate(val gometrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("RxPktRate"))
	return nil
}

// SetTxPktRate sets gauge in shared memory
func (mtr *ExampleMetrics) SetTxPktRate(val gometrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("TxPktRate"))
	return nil
}

// SetRxErrors sets cunter in shared memory
func (mtr *ExampleMetrics) SetRxErrors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxErrors"))
	return nil
}

// SetTxErrors sets cunter in shared memory
func (mtr *ExampleMetrics) SetTxErrors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxErrors"))
	return nil
}

// ExampleMetricsIterator is the iterator object
type ExampleMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *ExampleMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *ExampleMetricsIterator) Next() *ExampleMetrics {
	mtr := it.iter.Next()
	tmtr := &ExampleMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *ExampleMetricsIterator) Find(key uint32) (*ExampleMetrics, error) {

	mtr, err := it.iter.Find(string(proto.EncodeVarint(uint64(key))))

	if err != nil {
		return nil, err
	}
	tmtr := &ExampleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *ExampleMetricsIterator) Create(key uint32) (*ExampleMetrics, error) {
	tmtr := &ExampleMetrics{}

	mtr := it.iter.Create(string(proto.EncodeVarint(uint64(key))), tmtr.Size())

	tmtr = &ExampleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *ExampleMetricsIterator) Delete(key uint32) error {

	return it.iter.Delete(string(proto.EncodeVarint(uint64(key))))

}

// NewExampleMetricsIterator returns an iterator
func NewExampleMetricsIterator() (*ExampleMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("ExampleMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &ExampleMetricsIterator{iter: iter}, nil
}

type ExampleKey struct {
	Ifidx uint32
}

type NestedKeyExampleMetrics struct {
	key ExampleKey

	RxPkts gometrics.Counter

	TxPkts gometrics.Counter

	RxPktRate gometrics.Gauge

	TxPktRate gometrics.Gauge

	RxErrors gometrics.Counter

	TxErrors gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *NestedKeyExampleMetrics) GetKey() ExampleKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *NestedKeyExampleMetrics) Size() int {
	sz := 0

	sz += mtr.RxPkts.Size()

	sz += mtr.TxPkts.Size()

	sz += mtr.RxPktRate.Size()

	sz += mtr.TxPktRate.Size()

	sz += mtr.RxErrors.Size()

	sz += mtr.TxErrors.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *NestedKeyExampleMetrics) Unmarshal() error {
	var offset int

	json.Unmarshal([]byte(mtr.metrics.GetKey()), &mtr.key)

	mtr.RxPkts = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPkts.Size()

	mtr.TxPkts = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPkts.Size()

	mtr.RxPktRate = mtr.metrics.GetGauge(offset)
	offset += mtr.RxPktRate.Size()

	mtr.TxPktRate = mtr.metrics.GetGauge(offset)
	offset += mtr.TxPktRate.Size()

	mtr.RxErrors = mtr.metrics.GetCounter(offset)
	offset += mtr.RxErrors.Size()

	mtr.TxErrors = mtr.metrics.GetCounter(offset)
	offset += mtr.TxErrors.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *NestedKeyExampleMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxPkts" {
		return offset
	}
	offset += mtr.RxPkts.Size()

	if fldName == "TxPkts" {
		return offset
	}
	offset += mtr.TxPkts.Size()

	if fldName == "RxPktRate" {
		return offset
	}
	offset += mtr.RxPktRate.Size()

	if fldName == "TxPktRate" {
		return offset
	}
	offset += mtr.TxPktRate.Size()

	if fldName == "RxErrors" {
		return offset
	}
	offset += mtr.RxErrors.Size()

	if fldName == "TxErrors" {
		return offset
	}
	offset += mtr.TxErrors.Size()

	return offset
}

// SetRxPkts sets cunter in shared memory
func (mtr *NestedKeyExampleMetrics) SetRxPkts(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPkts"))
	return nil
}

// SetTxPkts sets cunter in shared memory
func (mtr *NestedKeyExampleMetrics) SetTxPkts(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPkts"))
	return nil
}

// SetRxPktRate sets gauge in shared memory
func (mtr *NestedKeyExampleMetrics) SetRxPktRate(val gometrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("RxPktRate"))
	return nil
}

// SetTxPktRate sets gauge in shared memory
func (mtr *NestedKeyExampleMetrics) SetTxPktRate(val gometrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("TxPktRate"))
	return nil
}

// SetRxErrors sets cunter in shared memory
func (mtr *NestedKeyExampleMetrics) SetRxErrors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxErrors"))
	return nil
}

// SetTxErrors sets cunter in shared memory
func (mtr *NestedKeyExampleMetrics) SetTxErrors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxErrors"))
	return nil
}

// NestedKeyExampleMetricsIterator is the iterator object
type NestedKeyExampleMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *NestedKeyExampleMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *NestedKeyExampleMetricsIterator) Next() *NestedKeyExampleMetrics {
	mtr := it.iter.Next()
	tmtr := &NestedKeyExampleMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *NestedKeyExampleMetricsIterator) Find(key ExampleKey) (*NestedKeyExampleMetrics, error) {

	buf, _ := json.Marshal(key)
	mtr, err := it.iter.Find(string(buf))

	if err != nil {
		return nil, err
	}
	tmtr := &NestedKeyExampleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *NestedKeyExampleMetricsIterator) Create(key ExampleKey) (*NestedKeyExampleMetrics, error) {
	tmtr := &NestedKeyExampleMetrics{}

	buf, _ := json.Marshal(key)
	mtr := it.iter.Create(string(buf), tmtr.Size())

	tmtr = &NestedKeyExampleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *NestedKeyExampleMetricsIterator) Delete(key ExampleKey) error {

	buf, _ := json.Marshal(key)
	return it.iter.Delete(string(buf))

}

// NewNestedKeyExampleMetricsIterator returns an iterator
func NewNestedKeyExampleMetricsIterator() (*NestedKeyExampleMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("NestedKeyExampleMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &NestedKeyExampleMetricsIterator{iter: iter}, nil
}

type SingletonExampleMetrics struct {
	key int

	RxPkts gometrics.Counter

	TxPkts gometrics.Counter

	RxPktRate gometrics.Gauge

	TxPktRate gometrics.Gauge

	RxErrors gometrics.Counter

	TxErrors gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *SingletonExampleMetrics) GetKey() int {
	return 0
}

// Size returns the size of the metrics object
func (mtr *SingletonExampleMetrics) Size() int {
	sz := 0

	sz += mtr.RxPkts.Size()

	sz += mtr.TxPkts.Size()

	sz += mtr.RxPktRate.Size()

	sz += mtr.TxPktRate.Size()

	sz += mtr.RxErrors.Size()

	sz += mtr.TxErrors.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *SingletonExampleMetrics) Unmarshal() error {
	var offset int

	mtr.RxPkts = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPkts.Size()

	mtr.TxPkts = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPkts.Size()

	mtr.RxPktRate = mtr.metrics.GetGauge(offset)
	offset += mtr.RxPktRate.Size()

	mtr.TxPktRate = mtr.metrics.GetGauge(offset)
	offset += mtr.TxPktRate.Size()

	mtr.RxErrors = mtr.metrics.GetCounter(offset)
	offset += mtr.RxErrors.Size()

	mtr.TxErrors = mtr.metrics.GetCounter(offset)
	offset += mtr.TxErrors.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *SingletonExampleMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxPkts" {
		return offset
	}
	offset += mtr.RxPkts.Size()

	if fldName == "TxPkts" {
		return offset
	}
	offset += mtr.TxPkts.Size()

	if fldName == "RxPktRate" {
		return offset
	}
	offset += mtr.RxPktRate.Size()

	if fldName == "TxPktRate" {
		return offset
	}
	offset += mtr.TxPktRate.Size()

	if fldName == "RxErrors" {
		return offset
	}
	offset += mtr.RxErrors.Size()

	if fldName == "TxErrors" {
		return offset
	}
	offset += mtr.TxErrors.Size()

	return offset
}

// SetRxPkts sets cunter in shared memory
func (mtr *SingletonExampleMetrics) SetRxPkts(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPkts"))
	return nil
}

// SetTxPkts sets cunter in shared memory
func (mtr *SingletonExampleMetrics) SetTxPkts(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPkts"))
	return nil
}

// SetRxPktRate sets gauge in shared memory
func (mtr *SingletonExampleMetrics) SetRxPktRate(val gometrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("RxPktRate"))
	return nil
}

// SetTxPktRate sets gauge in shared memory
func (mtr *SingletonExampleMetrics) SetTxPktRate(val gometrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("TxPktRate"))
	return nil
}

// SetRxErrors sets cunter in shared memory
func (mtr *SingletonExampleMetrics) SetRxErrors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxErrors"))
	return nil
}

// SetTxErrors sets cunter in shared memory
func (mtr *SingletonExampleMetrics) SetTxErrors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxErrors"))
	return nil
}

// SingletonExampleMetricsIterator is the iterator object
type SingletonExampleMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SingletonExampleMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SingletonExampleMetricsIterator) Next() *SingletonExampleMetrics {
	mtr := it.iter.Next()
	tmtr := &SingletonExampleMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SingletonExampleMetricsIterator) Find() (*SingletonExampleMetrics, error) {
	var key int
	mtr, err := it.iter.Find(string(proto.EncodeVarint(uint64(0))))

	if err != nil {
		return nil, err
	}
	tmtr := &SingletonExampleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SingletonExampleMetricsIterator) Create() (*SingletonExampleMetrics, error) {
	var key int
	tmtr := &SingletonExampleMetrics{}
	mtr := it.iter.Create(string(proto.EncodeVarint(uint64(0))), tmtr.Size())

	tmtr = &SingletonExampleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SingletonExampleMetricsIterator) Delete() error {
	return it.iter.Delete(string(proto.EncodeVarint(uint64(0))))

}

// NewSingletonExampleMetricsIterator returns an iterator
func NewSingletonExampleMetricsIterator() (*SingletonExampleMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SingletonExampleMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SingletonExampleMetricsIterator{iter: iter}, nil
}
