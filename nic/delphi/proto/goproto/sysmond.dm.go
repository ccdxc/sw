// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/ntsdb/metrics"
)

type AsicTemperatureMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	LocalTemperature metrics.Counter

	DieTemperature metrics.Counter

	HbmTemperature metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *AsicTemperatureMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *AsicTemperatureMetrics) Size() int {
	sz := 0

	sz += mtr.LocalTemperature.Size()

	sz += mtr.DieTemperature.Size()

	sz += mtr.HbmTemperature.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *AsicTemperatureMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.LocalTemperature = mtr.metrics.GetCounter(offset)
	offset += mtr.LocalTemperature.Size()

	mtr.DieTemperature = mtr.metrics.GetCounter(offset)
	offset += mtr.DieTemperature.Size()

	mtr.HbmTemperature = mtr.metrics.GetCounter(offset)
	offset += mtr.HbmTemperature.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *AsicTemperatureMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "LocalTemperature" {
		return offset
	}
	offset += mtr.LocalTemperature.Size()

	if fldName == "DieTemperature" {
		return offset
	}
	offset += mtr.DieTemperature.Size()

	if fldName == "HbmTemperature" {
		return offset
	}
	offset += mtr.HbmTemperature.Size()

	return offset
}

// SetLocalTemperature sets cunter in shared memory
func (mtr *AsicTemperatureMetrics) SetLocalTemperature(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LocalTemperature"))
	return nil
}

// SetDieTemperature sets cunter in shared memory
func (mtr *AsicTemperatureMetrics) SetDieTemperature(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DieTemperature"))
	return nil
}

// SetHbmTemperature sets cunter in shared memory
func (mtr *AsicTemperatureMetrics) SetHbmTemperature(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("HbmTemperature"))
	return nil
}

// AsicTemperatureMetricsIterator is the iterator object
type AsicTemperatureMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *AsicTemperatureMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *AsicTemperatureMetricsIterator) Next() *AsicTemperatureMetrics {
	mtr := it.iter.Next()
	tmtr := &AsicTemperatureMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *AsicTemperatureMetricsIterator) Find(key uint64) (*AsicTemperatureMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &AsicTemperatureMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *AsicTemperatureMetricsIterator) Create(key uint64) (*AsicTemperatureMetrics, error) {
	tmtr := &AsicTemperatureMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &AsicTemperatureMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *AsicTemperatureMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewAsicTemperatureMetricsIterator returns an iterator
func NewAsicTemperatureMetricsIterator() (*AsicTemperatureMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("AsicTemperatureMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &AsicTemperatureMetricsIterator{iter: iter}, nil
}

type AsicPowerMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Pin metrics.Counter

	Pout1 metrics.Counter

	Pout2 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *AsicPowerMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *AsicPowerMetrics) Size() int {
	sz := 0

	sz += mtr.Pin.Size()

	sz += mtr.Pout1.Size()

	sz += mtr.Pout2.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *AsicPowerMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Pin = mtr.metrics.GetCounter(offset)
	offset += mtr.Pin.Size()

	mtr.Pout1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pout1.Size()

	mtr.Pout2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pout2.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *AsicPowerMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Pin" {
		return offset
	}
	offset += mtr.Pin.Size()

	if fldName == "Pout1" {
		return offset
	}
	offset += mtr.Pout1.Size()

	if fldName == "Pout2" {
		return offset
	}
	offset += mtr.Pout2.Size()

	return offset
}

// SetPin sets cunter in shared memory
func (mtr *AsicPowerMetrics) SetPin(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pin"))
	return nil
}

// SetPout1 sets cunter in shared memory
func (mtr *AsicPowerMetrics) SetPout1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pout1"))
	return nil
}

// SetPout2 sets cunter in shared memory
func (mtr *AsicPowerMetrics) SetPout2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pout2"))
	return nil
}

// AsicPowerMetricsIterator is the iterator object
type AsicPowerMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *AsicPowerMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *AsicPowerMetricsIterator) Next() *AsicPowerMetrics {
	mtr := it.iter.Next()
	tmtr := &AsicPowerMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *AsicPowerMetricsIterator) Find(key uint64) (*AsicPowerMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &AsicPowerMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *AsicPowerMetricsIterator) Create(key uint64) (*AsicPowerMetrics, error) {
	tmtr := &AsicPowerMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &AsicPowerMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *AsicPowerMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewAsicPowerMetricsIterator returns an iterator
func NewAsicPowerMetricsIterator() (*AsicPowerMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("AsicPowerMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &AsicPowerMetricsIterator{iter: iter}, nil
}
