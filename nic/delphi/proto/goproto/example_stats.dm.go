// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/ntsdb/metrics"
)

type ExampleMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint32

	RxPkts metrics.Counter

	TxPkts metrics.Counter

	RxPktRate metrics.Gauge

	TxPktRate metrics.Gauge

	RxErrors metrics.Counter

	TxErrors metrics.Counter

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

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *ExampleMetrics) SetRxPkts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPkts"))
	return nil
}

// SetTxPkts sets cunter in shared memory
func (mtr *ExampleMetrics) SetTxPkts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPkts"))
	return nil
}

// SetRxPktRate sets gauge in shared memory
func (mtr *ExampleMetrics) SetRxPktRate(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("RxPktRate"))
	return nil
}

// SetTxPktRate sets gauge in shared memory
func (mtr *ExampleMetrics) SetTxPktRate(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("TxPktRate"))
	return nil
}

// SetRxErrors sets cunter in shared memory
func (mtr *ExampleMetrics) SetRxErrors(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxErrors"))
	return nil
}

// SetTxErrors sets cunter in shared memory
func (mtr *ExampleMetrics) SetTxErrors(val metrics.Counter) error {
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

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

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

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &ExampleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *ExampleMetricsIterator) Delete(key uint32) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

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
	Ifidx uint32 `protobuf:"varint,1,opt,name=Ifidx,json=Ifidx" json:"Ifidx,omitempty"`

	Lifid uint32 `protobuf:"varint,2,opt,name=Lifid,json=Lifid" json:"Lifid,omitempty"`
}

func (m *ExampleKey) Reset()         { *m = ExampleKey{} }
func (m *ExampleKey) String() string { return proto.CompactTextString(m) }
func (*ExampleKey) ProtoMessage()    {}

type NestedKeyExampleMetrics struct {
	ObjectMeta api.ObjectMeta

	key ExampleKey

	RxPkts metrics.Counter

	TxPkts metrics.Counter

	RxPktRate metrics.Gauge

	TxPktRate metrics.Gauge

	RxErrors metrics.Counter

	TxErrors metrics.Counter

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

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

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
func (mtr *NestedKeyExampleMetrics) SetRxPkts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPkts"))
	return nil
}

// SetTxPkts sets cunter in shared memory
func (mtr *NestedKeyExampleMetrics) SetTxPkts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPkts"))
	return nil
}

// SetRxPktRate sets gauge in shared memory
func (mtr *NestedKeyExampleMetrics) SetRxPktRate(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("RxPktRate"))
	return nil
}

// SetTxPktRate sets gauge in shared memory
func (mtr *NestedKeyExampleMetrics) SetTxPktRate(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("TxPktRate"))
	return nil
}

// SetRxErrors sets cunter in shared memory
func (mtr *NestedKeyExampleMetrics) SetRxErrors(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxErrors"))
	return nil
}

// SetTxErrors sets cunter in shared memory
func (mtr *NestedKeyExampleMetrics) SetTxErrors(val metrics.Counter) error {
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

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

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

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &NestedKeyExampleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *NestedKeyExampleMetricsIterator) Delete(key ExampleKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

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
	ObjectMeta api.ObjectMeta

	key int

	RxPkts metrics.Counter

	TxPkts metrics.Counter

	RxPktRate metrics.Gauge

	TxPktRate metrics.Gauge

	RxErrors metrics.Counter

	TxErrors metrics.Counter

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
func (mtr *SingletonExampleMetrics) SetRxPkts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPkts"))
	return nil
}

// SetTxPkts sets cunter in shared memory
func (mtr *SingletonExampleMetrics) SetTxPkts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPkts"))
	return nil
}

// SetRxPktRate sets gauge in shared memory
func (mtr *SingletonExampleMetrics) SetRxPktRate(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("RxPktRate"))
	return nil
}

// SetTxPktRate sets gauge in shared memory
func (mtr *SingletonExampleMetrics) SetTxPktRate(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("TxPktRate"))
	return nil
}

// SetRxErrors sets cunter in shared memory
func (mtr *SingletonExampleMetrics) SetRxErrors(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxErrors"))
	return nil
}

// SetTxErrors sets cunter in shared memory
func (mtr *SingletonExampleMetrics) SetTxErrors(val metrics.Counter) error {
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
	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(uint32(0)))

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
	mtr := it.iter.Create(gometrics.EncodeScalarKey(uint32(0)), tmtr.Size())

	tmtr = &SingletonExampleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SingletonExampleMetricsIterator) Delete() error {
	return it.iter.Delete(gometrics.EncodeScalarKey(uint32(0)))

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

type DpExampleMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint32

	RxPkts metrics.Counter

	TxPkts metrics.Counter

	RxPktRate metrics.Gauge

	TxPktRate metrics.Gauge

	RxErrors metrics.Counter

	TxErrors metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *DpExampleMetrics) GetKey() uint32 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *DpExampleMetrics) Size() int {
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
func (mtr *DpExampleMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

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
func (mtr *DpExampleMetrics) getOffset(fldName string) int {
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
func (mtr *DpExampleMetrics) SetRxPkts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPkts"))
	return nil
}

// SetTxPkts sets cunter in shared memory
func (mtr *DpExampleMetrics) SetTxPkts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPkts"))
	return nil
}

// SetRxPktRate sets gauge in shared memory
func (mtr *DpExampleMetrics) SetRxPktRate(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("RxPktRate"))
	return nil
}

// SetTxPktRate sets gauge in shared memory
func (mtr *DpExampleMetrics) SetTxPktRate(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("TxPktRate"))
	return nil
}

// SetRxErrors sets cunter in shared memory
func (mtr *DpExampleMetrics) SetRxErrors(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxErrors"))
	return nil
}

// SetTxErrors sets cunter in shared memory
func (mtr *DpExampleMetrics) SetTxErrors(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxErrors"))
	return nil
}

// DpExampleMetricsIterator is the iterator object
type DpExampleMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *DpExampleMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *DpExampleMetricsIterator) Next() *DpExampleMetrics {
	mtr := it.iter.Next()
	tmtr := &DpExampleMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *DpExampleMetricsIterator) Find(key uint32) (*DpExampleMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &DpExampleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *DpExampleMetricsIterator) Create(key uint32) (*DpExampleMetrics, error) {
	tmtr := &DpExampleMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &DpExampleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *DpExampleMetricsIterator) Delete(key uint32) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDpExampleMetricsIterator returns an iterator
func NewDpExampleMetricsIterator() (*DpExampleMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("DpExampleMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &DpExampleMetricsIterator{iter: iter}, nil
}

type NestedKeyDpExampleMetrics struct {
	ObjectMeta api.ObjectMeta

	key ExampleKey

	RxPkts metrics.Counter

	TxPkts metrics.Counter

	RxPktRate metrics.Gauge

	TxPktRate metrics.Gauge

	RxErrors metrics.Counter

	TxErrors metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *NestedKeyDpExampleMetrics) GetKey() ExampleKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *NestedKeyDpExampleMetrics) Size() int {
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
func (mtr *NestedKeyDpExampleMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

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
func (mtr *NestedKeyDpExampleMetrics) getOffset(fldName string) int {
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
func (mtr *NestedKeyDpExampleMetrics) SetRxPkts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPkts"))
	return nil
}

// SetTxPkts sets cunter in shared memory
func (mtr *NestedKeyDpExampleMetrics) SetTxPkts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPkts"))
	return nil
}

// SetRxPktRate sets gauge in shared memory
func (mtr *NestedKeyDpExampleMetrics) SetRxPktRate(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("RxPktRate"))
	return nil
}

// SetTxPktRate sets gauge in shared memory
func (mtr *NestedKeyDpExampleMetrics) SetTxPktRate(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("TxPktRate"))
	return nil
}

// SetRxErrors sets cunter in shared memory
func (mtr *NestedKeyDpExampleMetrics) SetRxErrors(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxErrors"))
	return nil
}

// SetTxErrors sets cunter in shared memory
func (mtr *NestedKeyDpExampleMetrics) SetTxErrors(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxErrors"))
	return nil
}

// NestedKeyDpExampleMetricsIterator is the iterator object
type NestedKeyDpExampleMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *NestedKeyDpExampleMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *NestedKeyDpExampleMetricsIterator) Next() *NestedKeyDpExampleMetrics {
	mtr := it.iter.Next()
	tmtr := &NestedKeyDpExampleMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *NestedKeyDpExampleMetricsIterator) Find(key ExampleKey) (*NestedKeyDpExampleMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &NestedKeyDpExampleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *NestedKeyDpExampleMetricsIterator) Create(key ExampleKey) (*NestedKeyDpExampleMetrics, error) {
	tmtr := &NestedKeyDpExampleMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &NestedKeyDpExampleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *NestedKeyDpExampleMetricsIterator) Delete(key ExampleKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// NewNestedKeyDpExampleMetricsIterator returns an iterator
func NewNestedKeyDpExampleMetricsIterator() (*NestedKeyDpExampleMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("NestedKeyDpExampleMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &NestedKeyDpExampleMetricsIterator{iter: iter}, nil
}
