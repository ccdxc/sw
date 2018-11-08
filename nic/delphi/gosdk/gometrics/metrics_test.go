// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package gometrics

import (
	"fmt"
	"testing"

	"github.com/golang/protobuf/proto"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type TestMetrics struct {
	key       uint32
	RxCounter Counter
	TxCounter Counter
	RxRate    Gauge
	TxRate    Gauge

	// private state
	metrics Metrics
}

func (mtr *TestMetrics) GetKey() uint32 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *TestMetrics) Size() int {
	sz := mtr.RxCounter.Size() +
		mtr.TxCounter.Size() +
		mtr.RxRate.Size() +
		mtr.TxRate.Size()

	return sz
}

// Unmarshall unmarshall the raw counters from shared memory
func (mtr *TestMetrics) Unmarshall() error {
	var offset int
	val, _ := proto.DecodeVarint([]byte(mtr.metrics.GetKey()))
	mtr.key = uint32(val)
	mtr.RxCounter = mtr.metrics.GetCounter(offset)
	offset += mtr.RxCounter.Size()
	mtr.TxCounter = mtr.metrics.GetCounter(offset)
	offset += mtr.TxCounter.Size()
	mtr.RxRate = mtr.metrics.GetGauge(offset)
	offset += mtr.RxRate.Size()
	mtr.TxRate = mtr.metrics.GetGauge(offset)
	offset += mtr.TxRate.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *TestMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxCounter" {
		return offset
	}
	offset += mtr.RxCounter.Size()
	if fldName == "TxCounter" {
		return offset
	}
	offset += mtr.TxCounter.Size()
	if fldName == "RxRate" {
		return offset
	}
	offset += mtr.RxRate.Size()
	if fldName == "TxRate" {
		return offset
	}
	offset += mtr.TxRate.Size()

	return offset
}

// SetRxCounter sets cunter in shared memory
func (mtr *TestMetrics) SetRxCounter(val Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxCounter"))
	return nil
}

// SetTxCounter sets counter in shared memory
func (mtr *TestMetrics) SetTxCounter(val Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxCounter"))
	return nil
}

// SetRxRate sets gauge in shared memory
func (mtr *TestMetrics) SetRxRate(val Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("RxRate"))
	return nil
}

// SetTxRate sets gause in shared memory
func (mtr *TestMetrics) SetTxRate(val Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("TxRate"))
	return nil
}

// TestMetricsIterator is the iterator object
type TestMetricsIterator struct {
	iter MetricsIterator
}

// HasNext returns true if there are more objects
func (it *TestMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *TestMetricsIterator) Next() *TestMetrics {
	mtr := it.iter.Next()
	tmtr := &TestMetrics{metrics: mtr}
	tmtr.Unmarshall()
	return tmtr
}

// Find finds the metrics object by key
func (it *TestMetricsIterator) Find(key uint32) (*TestMetrics, error) {
	mtr, err := it.iter.Find(string(proto.EncodeVarint(uint64(key))))
	if err != nil {
		return nil, err
	}
	tmtr := &TestMetrics{metrics: mtr, key: key}
	tmtr.Unmarshall()
	return tmtr, nil
}

// Create creates the object in shared memory
func (it *TestMetricsIterator) Create(key uint32) (*TestMetrics, error) {
	tmtr := &TestMetrics{}
	mtr := it.iter.Create(string(proto.EncodeVarint(uint64(key))), tmtr.Size())
	tmtr = &TestMetrics{metrics: mtr, key: key}
	tmtr.Unmarshall()
	return tmtr, nil
}

// Delete deletes the object from shared memory
func (it *TestMetricsIterator) Delete(key uint32) error {
	return it.iter.Delete(string(proto.EncodeVarint(uint64(key))))
}

// NewTestMetricsIterator returns an iterator
func NewTestMetricsIterator() (*TestMetricsIterator, error) {
	iter, err := NewMetricsIterator("TestMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &TestMetricsIterator{iter: iter}, nil
}

func TestMetricsGeneric(t *testing.T) {
	// create the iterator for the kind
	iter, err := NewMetricsIterator("TestMetrics")
	AssertOk(t, err, "Error creating metrics iterator")
	if iter == nil {
		// skip the test on osx
		return
	}

	// walk all metrics objects for the kind
	count := 0
	for iter.HasNext() {
		mtr := iter.Next()
		fmt.Printf("old Metrics: %v\n", mtr.String())
		count++
	}
	Assert(t, (count == 0), "Iterator found objects before create")

	// create an entry
	iter.Create("TestKey", 100)

	// find the key
	met, err := iter.Find("TestKey")
	AssertOk(t, err, "Error finding metrics entry")
	fmt.Printf("found Metrics: %v\n", met.String())

	// walk all metrics objects for the kind
	iter, _ = NewMetricsIterator("TestMetrics")
	count = 0
	for iter.HasNext() {
		mtr := iter.Next()
		fmt.Printf("new Metrics: %v\n", mtr.String())
		count++
	}
	Assert(t, (count == 1), "Iterator found invalid objects")

	// delete the entry
	iter.Delete("TestKey")

	// make sure we dont find it againa
	_, err = iter.Find("TestKey")
	Assert(t, (err != nil), "metrics find auceeded after delete")

	// make sure iterator doesnt find it
	count = 0
	iter, _ = NewMetricsIterator("TestMetrics")
	for iter.HasNext() {
		mtr := iter.Next()
		fmt.Printf("old Metrics: %v\n", mtr.String())
		count++
	}
	Assert(t, (count == 0), "Iterator found objects after delete")
}

func TestMetricsSpecific(t *testing.T) {
	iter, err := NewTestMetricsIterator()
	AssertOk(t, err, "Error creating metrics iterator")
	if iter == nil {
		// skip the test on osx
		return
	}

	// create an entry
	tmtr, err := iter.Create(3000)
	AssertOk(t, err, "Error creating test metrics entry")
	fmt.Printf("Created metrics: %+v\n", tmtr)

	// set some values
	tmtr.SetRxCounter(200)
	tmtr.SetTxCounter(300)
	tmtr.SetRxRate(400.0)
	tmtr.SetTxRate(500.0)

	tmtr, err = iter.Find(3000)
	AssertOk(t, err, "Error finding test metrics entry")
	fmt.Printf("Found metrics: %+v\n", tmtr)
	Assert(t, (tmtr.RxCounter == 200), "Invalid counter value")
	Assert(t, (tmtr.RxRate == 400.0), "Invalid gauge value")
	Assert(t, (tmtr.GetKey() == 3000), "Invalid key value")

	iter, err = NewTestMetricsIterator()
	count := 0
	for iter.HasNext() {
		mtr := iter.Next()
		fmt.Printf("new TestMetrics: %+v\n", mtr)
		Assert(t, (mtr.GetKey() == 3000), "Invalid key value")
		count++
	}
	Assert(t, (count == 1), "Iterator found invalid objects", count)

	// create second entry
	tmtr2, err := iter.Create(4000)
	AssertOk(t, err, "Error creating test metrics entry")

	tmtr2, err = iter.Find(4000)
	AssertOk(t, err, "Error creating test metrics entry")
	Assert(t, (tmtr2.GetKey() == 4000), "Invalid key value")

	// verify we could find both entries
	count = 0
	iter, err = NewTestMetricsIterator()
	for iter.HasNext() {
		mtr := iter.Next()
		fmt.Printf("new TestMetrics: %+v\n", mtr)
		Assert(t, ((mtr.GetKey() == 3000) || (mtr.GetKey() == 4000)), "Invalid key value")
		count++
	}
	Assert(t, (count == 2), "Iterator found invalid objects", count)

	// delete the metrics
	err = iter.Delete(3000)
	AssertOk(t, err, "Error deleting test metrics entry")

	_, err = iter.Find(3000)
	Assert(t, (err != nil), "metrics entry still found after deleting")

	tmtr2, err = iter.Find(4000)
	AssertOk(t, err, "Error creating test metrics entry")
	Assert(t, (tmtr2.GetKey() == 4000), "Invalid key value")

	// make sure iterator can still find the other entry
	count = 0
	iter, err = NewTestMetricsIterator()
	for iter.HasNext() {
		mtr := iter.Next()
		fmt.Printf("new TestMetrics: %+v\n", mtr)
		Assert(t, (mtr.GetKey() == 4000), "Invalid key value")
		count++
	}
	Assert(t, (count == 1), "Iterator found invalid objects", count)

	err = iter.Delete(4000)
	AssertOk(t, err, "Error deleting test metrics entry")

	_, err = iter.Find(4000)
	Assert(t, (err != nil), "metrics entry still found after deleting")
}
