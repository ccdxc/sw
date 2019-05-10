// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/tsdb/metrics"
)

type ruleMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	TcpHits metrics.Counter

	UdpHits metrics.Counter

	IcmpHits metrics.Counter

	EspHits metrics.Counter

	OtherHits metrics.Counter

	TotalHits metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *ruleMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *ruleMetrics) Size() int {
	sz := 0

	sz += mtr.TcpHits.Size()

	sz += mtr.UdpHits.Size()

	sz += mtr.IcmpHits.Size()

	sz += mtr.EspHits.Size()

	sz += mtr.OtherHits.Size()

	sz += mtr.TotalHits.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *ruleMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.TcpHits = mtr.metrics.GetCounter(offset)
	offset += mtr.TcpHits.Size()

	mtr.UdpHits = mtr.metrics.GetCounter(offset)
	offset += mtr.UdpHits.Size()

	mtr.IcmpHits = mtr.metrics.GetCounter(offset)
	offset += mtr.IcmpHits.Size()

	mtr.EspHits = mtr.metrics.GetCounter(offset)
	offset += mtr.EspHits.Size()

	mtr.OtherHits = mtr.metrics.GetCounter(offset)
	offset += mtr.OtherHits.Size()

	mtr.TotalHits = mtr.metrics.GetCounter(offset)
	offset += mtr.TotalHits.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *ruleMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "TcpHits" {
		return offset
	}
	offset += mtr.TcpHits.Size()

	if fldName == "UdpHits" {
		return offset
	}
	offset += mtr.UdpHits.Size()

	if fldName == "IcmpHits" {
		return offset
	}
	offset += mtr.IcmpHits.Size()

	if fldName == "EspHits" {
		return offset
	}
	offset += mtr.EspHits.Size()

	if fldName == "OtherHits" {
		return offset
	}
	offset += mtr.OtherHits.Size()

	if fldName == "TotalHits" {
		return offset
	}
	offset += mtr.TotalHits.Size()

	return offset
}

// SetTcpHits sets cunter in shared memory
func (mtr *ruleMetrics) SetTcpHits(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcpHits"))
	return nil
}

// SetUdpHits sets cunter in shared memory
func (mtr *ruleMetrics) SetUdpHits(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UdpHits"))
	return nil
}

// SetIcmpHits sets cunter in shared memory
func (mtr *ruleMetrics) SetIcmpHits(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IcmpHits"))
	return nil
}

// SetEspHits sets cunter in shared memory
func (mtr *ruleMetrics) SetEspHits(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EspHits"))
	return nil
}

// SetOtherHits sets cunter in shared memory
func (mtr *ruleMetrics) SetOtherHits(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OtherHits"))
	return nil
}

// SetTotalHits sets cunter in shared memory
func (mtr *ruleMetrics) SetTotalHits(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TotalHits"))
	return nil
}

// ruleMetricsIterator is the iterator object
type ruleMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *ruleMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *ruleMetricsIterator) Next() *ruleMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &ruleMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *ruleMetricsIterator) Find(key uint64) (*ruleMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &ruleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *ruleMetricsIterator) Create(key uint64) (*ruleMetrics, error) {
	tmtr := &ruleMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &ruleMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *ruleMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *ruleMetricsIterator) Free() {
	it.iter.Free()
}

// NewruleMetricsIterator returns an iterator
func NewruleMetricsIterator() (*ruleMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("ruleMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &ruleMetricsIterator{iter: iter}, nil
}
