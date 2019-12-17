// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/tsdb/metrics"
)

type AsicTemperatureMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	LocalTemperature metrics.Counter

	DieTemperature metrics.Counter

	HbmTemperature metrics.Counter

	QsfpPort1Temperature metrics.Counter

	QsfpPort2Temperature metrics.Counter

	QsfpPort1WarningTemperature metrics.Counter

	QsfpPort2WarningTemperature metrics.Counter

	QsfpPort1AlarmTemperature metrics.Counter

	QsfpPort2AlarmTemperature metrics.Counter

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

	sz += mtr.QsfpPort1Temperature.Size()

	sz += mtr.QsfpPort2Temperature.Size()

	sz += mtr.QsfpPort1WarningTemperature.Size()

	sz += mtr.QsfpPort2WarningTemperature.Size()

	sz += mtr.QsfpPort1AlarmTemperature.Size()

	sz += mtr.QsfpPort2AlarmTemperature.Size()

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

	mtr.QsfpPort1Temperature = mtr.metrics.GetCounter(offset)
	offset += mtr.QsfpPort1Temperature.Size()

	mtr.QsfpPort2Temperature = mtr.metrics.GetCounter(offset)
	offset += mtr.QsfpPort2Temperature.Size()

	mtr.QsfpPort1WarningTemperature = mtr.metrics.GetCounter(offset)
	offset += mtr.QsfpPort1WarningTemperature.Size()

	mtr.QsfpPort2WarningTemperature = mtr.metrics.GetCounter(offset)
	offset += mtr.QsfpPort2WarningTemperature.Size()

	mtr.QsfpPort1AlarmTemperature = mtr.metrics.GetCounter(offset)
	offset += mtr.QsfpPort1AlarmTemperature.Size()

	mtr.QsfpPort2AlarmTemperature = mtr.metrics.GetCounter(offset)
	offset += mtr.QsfpPort2AlarmTemperature.Size()

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

	if fldName == "QsfpPort1Temperature" {
		return offset
	}
	offset += mtr.QsfpPort1Temperature.Size()

	if fldName == "QsfpPort2Temperature" {
		return offset
	}
	offset += mtr.QsfpPort2Temperature.Size()

	if fldName == "QsfpPort1WarningTemperature" {
		return offset
	}
	offset += mtr.QsfpPort1WarningTemperature.Size()

	if fldName == "QsfpPort2WarningTemperature" {
		return offset
	}
	offset += mtr.QsfpPort2WarningTemperature.Size()

	if fldName == "QsfpPort1AlarmTemperature" {
		return offset
	}
	offset += mtr.QsfpPort1AlarmTemperature.Size()

	if fldName == "QsfpPort2AlarmTemperature" {
		return offset
	}
	offset += mtr.QsfpPort2AlarmTemperature.Size()

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

// SetQsfpPort1Temperature sets cunter in shared memory
func (mtr *AsicTemperatureMetrics) SetQsfpPort1Temperature(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QsfpPort1Temperature"))
	return nil
}

// SetQsfpPort2Temperature sets cunter in shared memory
func (mtr *AsicTemperatureMetrics) SetQsfpPort2Temperature(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QsfpPort2Temperature"))
	return nil
}

// SetQsfpPort1WarningTemperature sets cunter in shared memory
func (mtr *AsicTemperatureMetrics) SetQsfpPort1WarningTemperature(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QsfpPort1WarningTemperature"))
	return nil
}

// SetQsfpPort2WarningTemperature sets cunter in shared memory
func (mtr *AsicTemperatureMetrics) SetQsfpPort2WarningTemperature(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QsfpPort2WarningTemperature"))
	return nil
}

// SetQsfpPort1AlarmTemperature sets cunter in shared memory
func (mtr *AsicTemperatureMetrics) SetQsfpPort1AlarmTemperature(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QsfpPort1AlarmTemperature"))
	return nil
}

// SetQsfpPort2AlarmTemperature sets cunter in shared memory
func (mtr *AsicTemperatureMetrics) SetQsfpPort2AlarmTemperature(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QsfpPort2AlarmTemperature"))
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
	if mtr == nil {
		return nil
	}

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

// Free frees the iterator memory
func (it *AsicTemperatureMetricsIterator) Free() {
	it.iter.Free()
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
	if mtr == nil {
		return nil
	}

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

// Free frees the iterator memory
func (it *AsicPowerMetricsIterator) Free() {
	it.iter.Free()
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

type AsicFrequencyMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Frequency metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *AsicFrequencyMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *AsicFrequencyMetrics) Size() int {
	sz := 0

	sz += mtr.Frequency.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *AsicFrequencyMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Frequency = mtr.metrics.GetCounter(offset)
	offset += mtr.Frequency.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *AsicFrequencyMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Frequency" {
		return offset
	}
	offset += mtr.Frequency.Size()

	return offset
}

// SetFrequency sets cunter in shared memory
func (mtr *AsicFrequencyMetrics) SetFrequency(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Frequency"))
	return nil
}

// AsicFrequencyMetricsIterator is the iterator object
type AsicFrequencyMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *AsicFrequencyMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *AsicFrequencyMetricsIterator) Next() *AsicFrequencyMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &AsicFrequencyMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *AsicFrequencyMetricsIterator) Find(key uint64) (*AsicFrequencyMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &AsicFrequencyMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *AsicFrequencyMetricsIterator) Create(key uint64) (*AsicFrequencyMetrics, error) {
	tmtr := &AsicFrequencyMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &AsicFrequencyMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *AsicFrequencyMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *AsicFrequencyMetricsIterator) Free() {
	it.iter.Free()
}

// NewAsicFrequencyMetricsIterator returns an iterator
func NewAsicFrequencyMetricsIterator() (*AsicFrequencyMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("AsicFrequencyMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &AsicFrequencyMetricsIterator{iter: iter}, nil
}

type AsicMemoryMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Totalmemory metrics.Counter

	Availablememory metrics.Counter

	Freememory metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *AsicMemoryMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *AsicMemoryMetrics) Size() int {
	sz := 0

	sz += mtr.Totalmemory.Size()

	sz += mtr.Availablememory.Size()

	sz += mtr.Freememory.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *AsicMemoryMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Totalmemory = mtr.metrics.GetCounter(offset)
	offset += mtr.Totalmemory.Size()

	mtr.Availablememory = mtr.metrics.GetCounter(offset)
	offset += mtr.Availablememory.Size()

	mtr.Freememory = mtr.metrics.GetCounter(offset)
	offset += mtr.Freememory.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *AsicMemoryMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Totalmemory" {
		return offset
	}
	offset += mtr.Totalmemory.Size()

	if fldName == "Availablememory" {
		return offset
	}
	offset += mtr.Availablememory.Size()

	if fldName == "Freememory" {
		return offset
	}
	offset += mtr.Freememory.Size()

	return offset
}

// SetTotalmemory sets cunter in shared memory
func (mtr *AsicMemoryMetrics) SetTotalmemory(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Totalmemory"))
	return nil
}

// SetAvailablememory sets cunter in shared memory
func (mtr *AsicMemoryMetrics) SetAvailablememory(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Availablememory"))
	return nil
}

// SetFreememory sets cunter in shared memory
func (mtr *AsicMemoryMetrics) SetFreememory(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Freememory"))
	return nil
}

// AsicMemoryMetricsIterator is the iterator object
type AsicMemoryMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *AsicMemoryMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *AsicMemoryMetricsIterator) Next() *AsicMemoryMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &AsicMemoryMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *AsicMemoryMetricsIterator) Find(key uint64) (*AsicMemoryMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &AsicMemoryMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *AsicMemoryMetricsIterator) Create(key uint64) (*AsicMemoryMetrics, error) {
	tmtr := &AsicMemoryMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &AsicMemoryMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *AsicMemoryMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *AsicMemoryMetricsIterator) Free() {
	it.iter.Free()
}

// NewAsicMemoryMetricsIterator returns an iterator
func NewAsicMemoryMetricsIterator() (*AsicMemoryMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("AsicMemoryMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &AsicMemoryMetricsIterator{iter: iter}, nil
}
