// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/tsdb/metrics"
)

type PbpbchbmintecchbmhtMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbmintecchbmhtMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbmintecchbmhtMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbmintecchbmhtMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbmintecchbmhtMetrics) getOffset(fldName string) int {
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
func (mtr *PbpbchbmintecchbmhtMetrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *PbpbchbmintecchbmhtMetrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// PbpbchbmintecchbmhtMetricsIterator is the iterator object
type PbpbchbmintecchbmhtMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbmintecchbmhtMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbmintecchbmhtMetricsIterator) Next() *PbpbchbmintecchbmhtMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbchbmintecchbmhtMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbmintecchbmhtMetricsIterator) Find(key uint64) (*PbpbchbmintecchbmhtMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbmintecchbmhtMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbmintecchbmhtMetricsIterator) Create(key uint64) (*PbpbchbmintecchbmhtMetrics, error) {
	tmtr := &PbpbchbmintecchbmhtMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbmintecchbmhtMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbmintecchbmhtMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbchbmintecchbmhtMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbchbmintecchbmhtMetricsIterator returns an iterator
func NewPbpbchbmintecchbmhtMetricsIterator() (*PbpbchbmintecchbmhtMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbmintecchbmhtMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbmintecchbmhtMetricsIterator{iter: iter}, nil
}

type PbpbchbminthbmxoffMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Timeout_0 metrics.Counter

	Timeout_1 metrics.Counter

	Timeout_2 metrics.Counter

	Timeout_3 metrics.Counter

	Timeout_4 metrics.Counter

	Timeout_5 metrics.Counter

	Timeout_6 metrics.Counter

	Timeout_7 metrics.Counter

	Timeout_8 metrics.Counter

	Timeout_9 metrics.Counter

	Timeout_10 metrics.Counter

	Timeout_11 metrics.Counter

	Timeout_12 metrics.Counter

	Timeout_13 metrics.Counter

	Timeout_14 metrics.Counter

	Timeout_15 metrics.Counter

	Timeout_16 metrics.Counter

	Timeout_17 metrics.Counter

	Timeout_18 metrics.Counter

	Timeout_19 metrics.Counter

	Timeout_20 metrics.Counter

	Timeout_21 metrics.Counter

	Timeout_22 metrics.Counter

	Timeout_23 metrics.Counter

	Timeout_24 metrics.Counter

	Timeout_25 metrics.Counter

	Timeout_26 metrics.Counter

	Timeout_27 metrics.Counter

	Timeout_28 metrics.Counter

	Timeout_29 metrics.Counter

	Timeout_30 metrics.Counter

	Timeout_31 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbminthbmxoffMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbminthbmxoffMetrics) Size() int {
	sz := 0

	sz += mtr.Timeout_0.Size()

	sz += mtr.Timeout_1.Size()

	sz += mtr.Timeout_2.Size()

	sz += mtr.Timeout_3.Size()

	sz += mtr.Timeout_4.Size()

	sz += mtr.Timeout_5.Size()

	sz += mtr.Timeout_6.Size()

	sz += mtr.Timeout_7.Size()

	sz += mtr.Timeout_8.Size()

	sz += mtr.Timeout_9.Size()

	sz += mtr.Timeout_10.Size()

	sz += mtr.Timeout_11.Size()

	sz += mtr.Timeout_12.Size()

	sz += mtr.Timeout_13.Size()

	sz += mtr.Timeout_14.Size()

	sz += mtr.Timeout_15.Size()

	sz += mtr.Timeout_16.Size()

	sz += mtr.Timeout_17.Size()

	sz += mtr.Timeout_18.Size()

	sz += mtr.Timeout_19.Size()

	sz += mtr.Timeout_20.Size()

	sz += mtr.Timeout_21.Size()

	sz += mtr.Timeout_22.Size()

	sz += mtr.Timeout_23.Size()

	sz += mtr.Timeout_24.Size()

	sz += mtr.Timeout_25.Size()

	sz += mtr.Timeout_26.Size()

	sz += mtr.Timeout_27.Size()

	sz += mtr.Timeout_28.Size()

	sz += mtr.Timeout_29.Size()

	sz += mtr.Timeout_30.Size()

	sz += mtr.Timeout_31.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbminthbmxoffMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Timeout_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_0.Size()

	mtr.Timeout_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_1.Size()

	mtr.Timeout_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_2.Size()

	mtr.Timeout_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_3.Size()

	mtr.Timeout_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_4.Size()

	mtr.Timeout_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_5.Size()

	mtr.Timeout_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_6.Size()

	mtr.Timeout_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_7.Size()

	mtr.Timeout_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_8.Size()

	mtr.Timeout_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_9.Size()

	mtr.Timeout_10 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_10.Size()

	mtr.Timeout_11 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_11.Size()

	mtr.Timeout_12 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_12.Size()

	mtr.Timeout_13 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_13.Size()

	mtr.Timeout_14 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_14.Size()

	mtr.Timeout_15 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_15.Size()

	mtr.Timeout_16 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_16.Size()

	mtr.Timeout_17 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_17.Size()

	mtr.Timeout_18 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_18.Size()

	mtr.Timeout_19 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_19.Size()

	mtr.Timeout_20 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_20.Size()

	mtr.Timeout_21 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_21.Size()

	mtr.Timeout_22 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_22.Size()

	mtr.Timeout_23 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_23.Size()

	mtr.Timeout_24 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_24.Size()

	mtr.Timeout_25 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_25.Size()

	mtr.Timeout_26 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_26.Size()

	mtr.Timeout_27 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_27.Size()

	mtr.Timeout_28 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_28.Size()

	mtr.Timeout_29 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_29.Size()

	mtr.Timeout_30 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_30.Size()

	mtr.Timeout_31 = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout_31.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Timeout_0" {
		return offset
	}
	offset += mtr.Timeout_0.Size()

	if fldName == "Timeout_1" {
		return offset
	}
	offset += mtr.Timeout_1.Size()

	if fldName == "Timeout_2" {
		return offset
	}
	offset += mtr.Timeout_2.Size()

	if fldName == "Timeout_3" {
		return offset
	}
	offset += mtr.Timeout_3.Size()

	if fldName == "Timeout_4" {
		return offset
	}
	offset += mtr.Timeout_4.Size()

	if fldName == "Timeout_5" {
		return offset
	}
	offset += mtr.Timeout_5.Size()

	if fldName == "Timeout_6" {
		return offset
	}
	offset += mtr.Timeout_6.Size()

	if fldName == "Timeout_7" {
		return offset
	}
	offset += mtr.Timeout_7.Size()

	if fldName == "Timeout_8" {
		return offset
	}
	offset += mtr.Timeout_8.Size()

	if fldName == "Timeout_9" {
		return offset
	}
	offset += mtr.Timeout_9.Size()

	if fldName == "Timeout_10" {
		return offset
	}
	offset += mtr.Timeout_10.Size()

	if fldName == "Timeout_11" {
		return offset
	}
	offset += mtr.Timeout_11.Size()

	if fldName == "Timeout_12" {
		return offset
	}
	offset += mtr.Timeout_12.Size()

	if fldName == "Timeout_13" {
		return offset
	}
	offset += mtr.Timeout_13.Size()

	if fldName == "Timeout_14" {
		return offset
	}
	offset += mtr.Timeout_14.Size()

	if fldName == "Timeout_15" {
		return offset
	}
	offset += mtr.Timeout_15.Size()

	if fldName == "Timeout_16" {
		return offset
	}
	offset += mtr.Timeout_16.Size()

	if fldName == "Timeout_17" {
		return offset
	}
	offset += mtr.Timeout_17.Size()

	if fldName == "Timeout_18" {
		return offset
	}
	offset += mtr.Timeout_18.Size()

	if fldName == "Timeout_19" {
		return offset
	}
	offset += mtr.Timeout_19.Size()

	if fldName == "Timeout_20" {
		return offset
	}
	offset += mtr.Timeout_20.Size()

	if fldName == "Timeout_21" {
		return offset
	}
	offset += mtr.Timeout_21.Size()

	if fldName == "Timeout_22" {
		return offset
	}
	offset += mtr.Timeout_22.Size()

	if fldName == "Timeout_23" {
		return offset
	}
	offset += mtr.Timeout_23.Size()

	if fldName == "Timeout_24" {
		return offset
	}
	offset += mtr.Timeout_24.Size()

	if fldName == "Timeout_25" {
		return offset
	}
	offset += mtr.Timeout_25.Size()

	if fldName == "Timeout_26" {
		return offset
	}
	offset += mtr.Timeout_26.Size()

	if fldName == "Timeout_27" {
		return offset
	}
	offset += mtr.Timeout_27.Size()

	if fldName == "Timeout_28" {
		return offset
	}
	offset += mtr.Timeout_28.Size()

	if fldName == "Timeout_29" {
		return offset
	}
	offset += mtr.Timeout_29.Size()

	if fldName == "Timeout_30" {
		return offset
	}
	offset += mtr.Timeout_30.Size()

	if fldName == "Timeout_31" {
		return offset
	}
	offset += mtr.Timeout_31.Size()

	return offset
}

// SetTimeout_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_0"))
	return nil
}

// SetTimeout_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_1"))
	return nil
}

// SetTimeout_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_2"))
	return nil
}

// SetTimeout_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_3"))
	return nil
}

// SetTimeout_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_4"))
	return nil
}

// SetTimeout_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_5"))
	return nil
}

// SetTimeout_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_6"))
	return nil
}

// SetTimeout_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_7"))
	return nil
}

// SetTimeout_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_8"))
	return nil
}

// SetTimeout_9 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_9"))
	return nil
}

// SetTimeout_10 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_10"))
	return nil
}

// SetTimeout_11 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_11"))
	return nil
}

// SetTimeout_12 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_12(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_12"))
	return nil
}

// SetTimeout_13 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_13(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_13"))
	return nil
}

// SetTimeout_14 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_14(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_14"))
	return nil
}

// SetTimeout_15 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_15(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_15"))
	return nil
}

// SetTimeout_16 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_16(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_16"))
	return nil
}

// SetTimeout_17 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_17(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_17"))
	return nil
}

// SetTimeout_18 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_18(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_18"))
	return nil
}

// SetTimeout_19 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_19(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_19"))
	return nil
}

// SetTimeout_20 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_20(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_20"))
	return nil
}

// SetTimeout_21 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_21(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_21"))
	return nil
}

// SetTimeout_22 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_22(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_22"))
	return nil
}

// SetTimeout_23 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_23(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_23"))
	return nil
}

// SetTimeout_24 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_24(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_24"))
	return nil
}

// SetTimeout_25 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_25(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_25"))
	return nil
}

// SetTimeout_26 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_26(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_26"))
	return nil
}

// SetTimeout_27 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_27(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_27"))
	return nil
}

// SetTimeout_28 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_28(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_28"))
	return nil
}

// SetTimeout_29 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_29(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_29"))
	return nil
}

// SetTimeout_30 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_30(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_30"))
	return nil
}

// SetTimeout_31 sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout_31(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout_31"))
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
	if mtr == nil {
		return nil
	}

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

// Free frees the iterator memory
func (it *PbpbchbminthbmxoffMetricsIterator) Free() {
	it.iter.Free()
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

type PbpbchbminthbmpbusviolationinMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SopSopIn_0 metrics.Counter

	SopSopIn_1 metrics.Counter

	SopSopIn_2 metrics.Counter

	SopSopIn_3 metrics.Counter

	SopSopIn_4 metrics.Counter

	SopSopIn_5 metrics.Counter

	SopSopIn_6 metrics.Counter

	SopSopIn_7 metrics.Counter

	SopSopIn_8 metrics.Counter

	SopSopIn_9 metrics.Counter

	EopEopIn_0 metrics.Counter

	EopEopIn_1 metrics.Counter

	EopEopIn_2 metrics.Counter

	EopEopIn_3 metrics.Counter

	EopEopIn_4 metrics.Counter

	EopEopIn_5 metrics.Counter

	EopEopIn_6 metrics.Counter

	EopEopIn_7 metrics.Counter

	EopEopIn_8 metrics.Counter

	EopEopIn_9 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbminthbmpbusviolationinMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbminthbmpbusviolationinMetrics) Size() int {
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
func (mtr *PbpbchbminthbmpbusviolationinMetrics) Unmarshal() error {
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
func (mtr *PbpbchbminthbmpbusviolationinMetrics) getOffset(fldName string) int {
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
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetSopSopIn_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_0"))
	return nil
}

// SetSopSopIn_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetSopSopIn_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_1"))
	return nil
}

// SetSopSopIn_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetSopSopIn_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_2"))
	return nil
}

// SetSopSopIn_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetSopSopIn_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_3"))
	return nil
}

// SetSopSopIn_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetSopSopIn_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_4"))
	return nil
}

// SetSopSopIn_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetSopSopIn_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_5"))
	return nil
}

// SetSopSopIn_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetSopSopIn_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_6"))
	return nil
}

// SetSopSopIn_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetSopSopIn_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_7"))
	return nil
}

// SetSopSopIn_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetSopSopIn_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_8"))
	return nil
}

// SetSopSopIn_9 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetSopSopIn_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_9"))
	return nil
}

// SetEopEopIn_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetEopEopIn_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_0"))
	return nil
}

// SetEopEopIn_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetEopEopIn_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_1"))
	return nil
}

// SetEopEopIn_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetEopEopIn_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_2"))
	return nil
}

// SetEopEopIn_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetEopEopIn_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_3"))
	return nil
}

// SetEopEopIn_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetEopEopIn_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_4"))
	return nil
}

// SetEopEopIn_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetEopEopIn_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_5"))
	return nil
}

// SetEopEopIn_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetEopEopIn_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_6"))
	return nil
}

// SetEopEopIn_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetEopEopIn_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_7"))
	return nil
}

// SetEopEopIn_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetEopEopIn_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_8"))
	return nil
}

// SetEopEopIn_9 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationinMetrics) SetEopEopIn_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_9"))
	return nil
}

// PbpbchbminthbmpbusviolationinMetricsIterator is the iterator object
type PbpbchbminthbmpbusviolationinMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbminthbmpbusviolationinMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbminthbmpbusviolationinMetricsIterator) Next() *PbpbchbminthbmpbusviolationinMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbchbminthbmpbusviolationinMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbminthbmpbusviolationinMetricsIterator) Find(key uint64) (*PbpbchbminthbmpbusviolationinMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbminthbmpbusviolationinMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbminthbmpbusviolationinMetricsIterator) Create(key uint64) (*PbpbchbminthbmpbusviolationinMetrics, error) {
	tmtr := &PbpbchbminthbmpbusviolationinMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbminthbmpbusviolationinMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbminthbmpbusviolationinMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbchbminthbmpbusviolationinMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbchbminthbmpbusviolationinMetricsIterator returns an iterator
func NewPbpbchbminthbmpbusviolationinMetricsIterator() (*PbpbchbminthbmpbusviolationinMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbminthbmpbusviolationinMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbminthbmpbusviolationinMetricsIterator{iter: iter}, nil
}

type PbpbchbminthbmpbusviolationoutMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SopSopOut_0 metrics.Counter

	SopSopOut_1 metrics.Counter

	SopSopOut_2 metrics.Counter

	SopSopOut_3 metrics.Counter

	SopSopOut_4 metrics.Counter

	SopSopOut_5 metrics.Counter

	SopSopOut_6 metrics.Counter

	SopSopOut_7 metrics.Counter

	SopSopOut_8 metrics.Counter

	SopSopOut_9 metrics.Counter

	EopEopOut_0 metrics.Counter

	EopEopOut_1 metrics.Counter

	EopEopOut_2 metrics.Counter

	EopEopOut_3 metrics.Counter

	EopEopOut_4 metrics.Counter

	EopEopOut_5 metrics.Counter

	EopEopOut_6 metrics.Counter

	EopEopOut_7 metrics.Counter

	EopEopOut_8 metrics.Counter

	EopEopOut_9 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbminthbmpbusviolationoutMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) Size() int {
	sz := 0

	sz += mtr.SopSopOut_0.Size()

	sz += mtr.SopSopOut_1.Size()

	sz += mtr.SopSopOut_2.Size()

	sz += mtr.SopSopOut_3.Size()

	sz += mtr.SopSopOut_4.Size()

	sz += mtr.SopSopOut_5.Size()

	sz += mtr.SopSopOut_6.Size()

	sz += mtr.SopSopOut_7.Size()

	sz += mtr.SopSopOut_8.Size()

	sz += mtr.SopSopOut_9.Size()

	sz += mtr.EopEopOut_0.Size()

	sz += mtr.EopEopOut_1.Size()

	sz += mtr.EopEopOut_2.Size()

	sz += mtr.EopEopOut_3.Size()

	sz += mtr.EopEopOut_4.Size()

	sz += mtr.EopEopOut_5.Size()

	sz += mtr.EopEopOut_6.Size()

	sz += mtr.EopEopOut_7.Size()

	sz += mtr.EopEopOut_8.Size()

	sz += mtr.EopEopOut_9.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SopSopOut_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopOut_0.Size()

	mtr.SopSopOut_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopOut_1.Size()

	mtr.SopSopOut_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopOut_2.Size()

	mtr.SopSopOut_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopOut_3.Size()

	mtr.SopSopOut_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopOut_4.Size()

	mtr.SopSopOut_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopOut_5.Size()

	mtr.SopSopOut_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopOut_6.Size()

	mtr.SopSopOut_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopOut_7.Size()

	mtr.SopSopOut_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopOut_8.Size()

	mtr.SopSopOut_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopOut_9.Size()

	mtr.EopEopOut_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopOut_0.Size()

	mtr.EopEopOut_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopOut_1.Size()

	mtr.EopEopOut_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopOut_2.Size()

	mtr.EopEopOut_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopOut_3.Size()

	mtr.EopEopOut_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopOut_4.Size()

	mtr.EopEopOut_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopOut_5.Size()

	mtr.EopEopOut_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopOut_6.Size()

	mtr.EopEopOut_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopOut_7.Size()

	mtr.EopEopOut_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopOut_8.Size()

	mtr.EopEopOut_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopOut_9.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SopSopOut_0" {
		return offset
	}
	offset += mtr.SopSopOut_0.Size()

	if fldName == "SopSopOut_1" {
		return offset
	}
	offset += mtr.SopSopOut_1.Size()

	if fldName == "SopSopOut_2" {
		return offset
	}
	offset += mtr.SopSopOut_2.Size()

	if fldName == "SopSopOut_3" {
		return offset
	}
	offset += mtr.SopSopOut_3.Size()

	if fldName == "SopSopOut_4" {
		return offset
	}
	offset += mtr.SopSopOut_4.Size()

	if fldName == "SopSopOut_5" {
		return offset
	}
	offset += mtr.SopSopOut_5.Size()

	if fldName == "SopSopOut_6" {
		return offset
	}
	offset += mtr.SopSopOut_6.Size()

	if fldName == "SopSopOut_7" {
		return offset
	}
	offset += mtr.SopSopOut_7.Size()

	if fldName == "SopSopOut_8" {
		return offset
	}
	offset += mtr.SopSopOut_8.Size()

	if fldName == "SopSopOut_9" {
		return offset
	}
	offset += mtr.SopSopOut_9.Size()

	if fldName == "EopEopOut_0" {
		return offset
	}
	offset += mtr.EopEopOut_0.Size()

	if fldName == "EopEopOut_1" {
		return offset
	}
	offset += mtr.EopEopOut_1.Size()

	if fldName == "EopEopOut_2" {
		return offset
	}
	offset += mtr.EopEopOut_2.Size()

	if fldName == "EopEopOut_3" {
		return offset
	}
	offset += mtr.EopEopOut_3.Size()

	if fldName == "EopEopOut_4" {
		return offset
	}
	offset += mtr.EopEopOut_4.Size()

	if fldName == "EopEopOut_5" {
		return offset
	}
	offset += mtr.EopEopOut_5.Size()

	if fldName == "EopEopOut_6" {
		return offset
	}
	offset += mtr.EopEopOut_6.Size()

	if fldName == "EopEopOut_7" {
		return offset
	}
	offset += mtr.EopEopOut_7.Size()

	if fldName == "EopEopOut_8" {
		return offset
	}
	offset += mtr.EopEopOut_8.Size()

	if fldName == "EopEopOut_9" {
		return offset
	}
	offset += mtr.EopEopOut_9.Size()

	return offset
}

// SetSopSopOut_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetSopSopOut_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_0"))
	return nil
}

// SetSopSopOut_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetSopSopOut_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_1"))
	return nil
}

// SetSopSopOut_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetSopSopOut_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_2"))
	return nil
}

// SetSopSopOut_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetSopSopOut_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_3"))
	return nil
}

// SetSopSopOut_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetSopSopOut_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_4"))
	return nil
}

// SetSopSopOut_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetSopSopOut_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_5"))
	return nil
}

// SetSopSopOut_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetSopSopOut_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_6"))
	return nil
}

// SetSopSopOut_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetSopSopOut_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_7"))
	return nil
}

// SetSopSopOut_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetSopSopOut_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_8"))
	return nil
}

// SetSopSopOut_9 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetSopSopOut_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_9"))
	return nil
}

// SetEopEopOut_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetEopEopOut_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_0"))
	return nil
}

// SetEopEopOut_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetEopEopOut_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_1"))
	return nil
}

// SetEopEopOut_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetEopEopOut_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_2"))
	return nil
}

// SetEopEopOut_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetEopEopOut_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_3"))
	return nil
}

// SetEopEopOut_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetEopEopOut_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_4"))
	return nil
}

// SetEopEopOut_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetEopEopOut_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_5"))
	return nil
}

// SetEopEopOut_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetEopEopOut_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_6"))
	return nil
}

// SetEopEopOut_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetEopEopOut_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_7"))
	return nil
}

// SetEopEopOut_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetEopEopOut_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_8"))
	return nil
}

// SetEopEopOut_9 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationoutMetrics) SetEopEopOut_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_9"))
	return nil
}

// PbpbchbminthbmpbusviolationoutMetricsIterator is the iterator object
type PbpbchbminthbmpbusviolationoutMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbminthbmpbusviolationoutMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbminthbmpbusviolationoutMetricsIterator) Next() *PbpbchbminthbmpbusviolationoutMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbchbminthbmpbusviolationoutMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbminthbmpbusviolationoutMetricsIterator) Find(key uint64) (*PbpbchbminthbmpbusviolationoutMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbminthbmpbusviolationoutMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbminthbmpbusviolationoutMetricsIterator) Create(key uint64) (*PbpbchbminthbmpbusviolationoutMetrics, error) {
	tmtr := &PbpbchbminthbmpbusviolationoutMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbminthbmpbusviolationoutMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbminthbmpbusviolationoutMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbchbminthbmpbusviolationoutMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbchbminthbmpbusviolationoutMetricsIterator returns an iterator
func NewPbpbchbminthbmpbusviolationoutMetricsIterator() (*PbpbchbminthbmpbusviolationoutMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbminthbmpbusviolationoutMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbminthbmpbusviolationoutMetricsIterator{iter: iter}, nil
}

type PbpbchbminthbmdropMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Occupancy_0 metrics.Counter

	Occupancy_1 metrics.Counter

	Occupancy_2 metrics.Counter

	Occupancy_3 metrics.Counter

	Occupancy_4 metrics.Counter

	Occupancy_5 metrics.Counter

	Occupancy_6 metrics.Counter

	Occupancy_7 metrics.Counter

	Occupancy_8 metrics.Counter

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

	return offset
}

// SetOccupancy_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_0"))
	return nil
}

// SetOccupancy_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_1"))
	return nil
}

// SetOccupancy_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_2"))
	return nil
}

// SetOccupancy_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_3"))
	return nil
}

// SetOccupancy_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_4"))
	return nil
}

// SetOccupancy_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_5"))
	return nil
}

// SetOccupancy_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_6"))
	return nil
}

// SetOccupancy_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_7"))
	return nil
}

// SetOccupancy_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_8"))
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
	if mtr == nil {
		return nil
	}

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

// Free frees the iterator memory
func (it *PbpbchbminthbmdropMetricsIterator) Free() {
	it.iter.Free()
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

type PbpbchbminthbmstopMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Occupancy_9 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbminthbmstopMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbminthbmstopMetrics) Size() int {
	sz := 0

	sz += mtr.Occupancy_9.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbminthbmstopMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Occupancy_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.Occupancy_9.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbminthbmstopMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Occupancy_9" {
		return offset
	}
	offset += mtr.Occupancy_9.Size()

	return offset
}

// SetOccupancy_9 sets cunter in shared memory
func (mtr *PbpbchbminthbmstopMetrics) SetOccupancy_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_9"))
	return nil
}

// PbpbchbminthbmstopMetricsIterator is the iterator object
type PbpbchbminthbmstopMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbminthbmstopMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbminthbmstopMetricsIterator) Next() *PbpbchbminthbmstopMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbchbminthbmstopMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbminthbmstopMetricsIterator) Find(key uint64) (*PbpbchbminthbmstopMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbminthbmstopMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbminthbmstopMetricsIterator) Create(key uint64) (*PbpbchbminthbmstopMetrics, error) {
	tmtr := &PbpbchbminthbmstopMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbminthbmstopMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbminthbmstopMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbchbminthbmstopMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbchbminthbmstopMetricsIterator returns an iterator
func NewPbpbchbminthbmstopMetricsIterator() (*PbpbchbminthbmstopMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbminthbmstopMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbminthbmstopMetricsIterator{iter: iter}, nil
}

type PbpbchbminthbmaxierrrspMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Ctrl metrics.Counter

	Pyld metrics.Counter

	R2A metrics.Counter

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
func (mtr *PbpbchbminthbmaxierrrspMetrics) SetCtrl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ctrl"))
	return nil
}

// SetPyld sets cunter in shared memory
func (mtr *PbpbchbminthbmaxierrrspMetrics) SetPyld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pyld"))
	return nil
}

// SetR2A sets cunter in shared memory
func (mtr *PbpbchbminthbmaxierrrspMetrics) SetR2A(val metrics.Counter) error {
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
	if mtr == nil {
		return nil
	}

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

// Free frees the iterator memory
func (it *PbpbchbminthbmaxierrrspMetricsIterator) Free() {
	it.iter.Free()
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

type PbpbchbminthbmdropemergencyMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Stop_0 metrics.Counter

	Stop_1 metrics.Counter

	Stop_2 metrics.Counter

	Stop_3 metrics.Counter

	Stop_4 metrics.Counter

	Stop_5 metrics.Counter

	Stop_6 metrics.Counter

	Stop_7 metrics.Counter

	Stop_8 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbminthbmdropemergencyMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbminthbmdropemergencyMetrics) Size() int {
	sz := 0

	sz += mtr.Stop_0.Size()

	sz += mtr.Stop_1.Size()

	sz += mtr.Stop_2.Size()

	sz += mtr.Stop_3.Size()

	sz += mtr.Stop_4.Size()

	sz += mtr.Stop_5.Size()

	sz += mtr.Stop_6.Size()

	sz += mtr.Stop_7.Size()

	sz += mtr.Stop_8.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbminthbmdropemergencyMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Stop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Stop_0.Size()

	mtr.Stop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Stop_1.Size()

	mtr.Stop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Stop_2.Size()

	mtr.Stop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Stop_3.Size()

	mtr.Stop_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.Stop_4.Size()

	mtr.Stop_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.Stop_5.Size()

	mtr.Stop_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.Stop_6.Size()

	mtr.Stop_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.Stop_7.Size()

	mtr.Stop_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.Stop_8.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbminthbmdropemergencyMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Stop_0" {
		return offset
	}
	offset += mtr.Stop_0.Size()

	if fldName == "Stop_1" {
		return offset
	}
	offset += mtr.Stop_1.Size()

	if fldName == "Stop_2" {
		return offset
	}
	offset += mtr.Stop_2.Size()

	if fldName == "Stop_3" {
		return offset
	}
	offset += mtr.Stop_3.Size()

	if fldName == "Stop_4" {
		return offset
	}
	offset += mtr.Stop_4.Size()

	if fldName == "Stop_5" {
		return offset
	}
	offset += mtr.Stop_5.Size()

	if fldName == "Stop_6" {
		return offset
	}
	offset += mtr.Stop_6.Size()

	if fldName == "Stop_7" {
		return offset
	}
	offset += mtr.Stop_7.Size()

	if fldName == "Stop_8" {
		return offset
	}
	offset += mtr.Stop_8.Size()

	return offset
}

// SetStop_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropemergencyMetrics) SetStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Stop_0"))
	return nil
}

// SetStop_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropemergencyMetrics) SetStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Stop_1"))
	return nil
}

// SetStop_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropemergencyMetrics) SetStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Stop_2"))
	return nil
}

// SetStop_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropemergencyMetrics) SetStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Stop_3"))
	return nil
}

// SetStop_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropemergencyMetrics) SetStop_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Stop_4"))
	return nil
}

// SetStop_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropemergencyMetrics) SetStop_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Stop_5"))
	return nil
}

// SetStop_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropemergencyMetrics) SetStop_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Stop_6"))
	return nil
}

// SetStop_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropemergencyMetrics) SetStop_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Stop_7"))
	return nil
}

// SetStop_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropemergencyMetrics) SetStop_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Stop_8"))
	return nil
}

// PbpbchbminthbmdropemergencyMetricsIterator is the iterator object
type PbpbchbminthbmdropemergencyMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbminthbmdropemergencyMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbminthbmdropemergencyMetricsIterator) Next() *PbpbchbminthbmdropemergencyMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbchbminthbmdropemergencyMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbminthbmdropemergencyMetricsIterator) Find(key uint64) (*PbpbchbminthbmdropemergencyMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbminthbmdropemergencyMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbminthbmdropemergencyMetricsIterator) Create(key uint64) (*PbpbchbminthbmdropemergencyMetrics, error) {
	tmtr := &PbpbchbminthbmdropemergencyMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbminthbmdropemergencyMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbminthbmdropemergencyMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbchbminthbmdropemergencyMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbchbminthbmdropemergencyMetricsIterator returns an iterator
func NewPbpbchbminthbmdropemergencyMetricsIterator() (*PbpbchbminthbmdropemergencyMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbminthbmdropemergencyMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbminthbmdropemergencyMetricsIterator{iter: iter}, nil
}

type PbpbchbminthbmdropwriteackMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	FillingUp_0 metrics.Counter

	FillingUp_1 metrics.Counter

	FillingUp_2 metrics.Counter

	FillingUp_3 metrics.Counter

	FillingUp_4 metrics.Counter

	FillingUp_5 metrics.Counter

	FillingUp_6 metrics.Counter

	FillingUp_7 metrics.Counter

	FillingUp_8 metrics.Counter

	Full_0 metrics.Counter

	Full_1 metrics.Counter

	Full_2 metrics.Counter

	Full_3 metrics.Counter

	Full_4 metrics.Counter

	Full_5 metrics.Counter

	Full_6 metrics.Counter

	Full_7 metrics.Counter

	Full_8 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbminthbmdropwriteackMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbminthbmdropwriteackMetrics) Size() int {
	sz := 0

	sz += mtr.FillingUp_0.Size()

	sz += mtr.FillingUp_1.Size()

	sz += mtr.FillingUp_2.Size()

	sz += mtr.FillingUp_3.Size()

	sz += mtr.FillingUp_4.Size()

	sz += mtr.FillingUp_5.Size()

	sz += mtr.FillingUp_6.Size()

	sz += mtr.FillingUp_7.Size()

	sz += mtr.FillingUp_8.Size()

	sz += mtr.Full_0.Size()

	sz += mtr.Full_1.Size()

	sz += mtr.Full_2.Size()

	sz += mtr.Full_3.Size()

	sz += mtr.Full_4.Size()

	sz += mtr.Full_5.Size()

	sz += mtr.Full_6.Size()

	sz += mtr.Full_7.Size()

	sz += mtr.Full_8.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.FillingUp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.FillingUp_0.Size()

	mtr.FillingUp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.FillingUp_1.Size()

	mtr.FillingUp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.FillingUp_2.Size()

	mtr.FillingUp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.FillingUp_3.Size()

	mtr.FillingUp_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.FillingUp_4.Size()

	mtr.FillingUp_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.FillingUp_5.Size()

	mtr.FillingUp_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.FillingUp_6.Size()

	mtr.FillingUp_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.FillingUp_7.Size()

	mtr.FillingUp_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.FillingUp_8.Size()

	mtr.Full_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_0.Size()

	mtr.Full_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_1.Size()

	mtr.Full_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_2.Size()

	mtr.Full_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_3.Size()

	mtr.Full_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_4.Size()

	mtr.Full_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_5.Size()

	mtr.Full_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_6.Size()

	mtr.Full_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_7.Size()

	mtr.Full_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_8.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "FillingUp_0" {
		return offset
	}
	offset += mtr.FillingUp_0.Size()

	if fldName == "FillingUp_1" {
		return offset
	}
	offset += mtr.FillingUp_1.Size()

	if fldName == "FillingUp_2" {
		return offset
	}
	offset += mtr.FillingUp_2.Size()

	if fldName == "FillingUp_3" {
		return offset
	}
	offset += mtr.FillingUp_3.Size()

	if fldName == "FillingUp_4" {
		return offset
	}
	offset += mtr.FillingUp_4.Size()

	if fldName == "FillingUp_5" {
		return offset
	}
	offset += mtr.FillingUp_5.Size()

	if fldName == "FillingUp_6" {
		return offset
	}
	offset += mtr.FillingUp_6.Size()

	if fldName == "FillingUp_7" {
		return offset
	}
	offset += mtr.FillingUp_7.Size()

	if fldName == "FillingUp_8" {
		return offset
	}
	offset += mtr.FillingUp_8.Size()

	if fldName == "Full_0" {
		return offset
	}
	offset += mtr.Full_0.Size()

	if fldName == "Full_1" {
		return offset
	}
	offset += mtr.Full_1.Size()

	if fldName == "Full_2" {
		return offset
	}
	offset += mtr.Full_2.Size()

	if fldName == "Full_3" {
		return offset
	}
	offset += mtr.Full_3.Size()

	if fldName == "Full_4" {
		return offset
	}
	offset += mtr.Full_4.Size()

	if fldName == "Full_5" {
		return offset
	}
	offset += mtr.Full_5.Size()

	if fldName == "Full_6" {
		return offset
	}
	offset += mtr.Full_6.Size()

	if fldName == "Full_7" {
		return offset
	}
	offset += mtr.Full_7.Size()

	if fldName == "Full_8" {
		return offset
	}
	offset += mtr.Full_8.Size()

	return offset
}

// SetFillingUp_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFillingUp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FillingUp_0"))
	return nil
}

// SetFillingUp_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFillingUp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FillingUp_1"))
	return nil
}

// SetFillingUp_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFillingUp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FillingUp_2"))
	return nil
}

// SetFillingUp_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFillingUp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FillingUp_3"))
	return nil
}

// SetFillingUp_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFillingUp_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FillingUp_4"))
	return nil
}

// SetFillingUp_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFillingUp_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FillingUp_5"))
	return nil
}

// SetFillingUp_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFillingUp_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FillingUp_6"))
	return nil
}

// SetFillingUp_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFillingUp_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FillingUp_7"))
	return nil
}

// SetFillingUp_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFillingUp_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FillingUp_8"))
	return nil
}

// SetFull_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFull_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_0"))
	return nil
}

// SetFull_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFull_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_1"))
	return nil
}

// SetFull_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFull_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_2"))
	return nil
}

// SetFull_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFull_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_3"))
	return nil
}

// SetFull_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFull_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_4"))
	return nil
}

// SetFull_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFull_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_5"))
	return nil
}

// SetFull_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFull_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_6"))
	return nil
}

// SetFull_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFull_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_7"))
	return nil
}

// SetFull_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropwriteackMetrics) SetFull_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_8"))
	return nil
}

// PbpbchbminthbmdropwriteackMetricsIterator is the iterator object
type PbpbchbminthbmdropwriteackMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbminthbmdropwriteackMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbminthbmdropwriteackMetricsIterator) Next() *PbpbchbminthbmdropwriteackMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbchbminthbmdropwriteackMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbminthbmdropwriteackMetricsIterator) Find(key uint64) (*PbpbchbminthbmdropwriteackMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbminthbmdropwriteackMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbminthbmdropwriteackMetricsIterator) Create(key uint64) (*PbpbchbminthbmdropwriteackMetrics, error) {
	tmtr := &PbpbchbminthbmdropwriteackMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbminthbmdropwriteackMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbminthbmdropwriteackMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbchbminthbmdropwriteackMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbchbminthbmdropwriteackMetricsIterator returns an iterator
func NewPbpbchbminthbmdropwriteackMetricsIterator() (*PbpbchbminthbmdropwriteackMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbminthbmdropwriteackMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbminthbmdropwriteackMetricsIterator{iter: iter}, nil
}

type PbpbchbminthbmdropctrlMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Full_0 metrics.Counter

	Full_1 metrics.Counter

	Full_2 metrics.Counter

	Full_3 metrics.Counter

	Full_4 metrics.Counter

	Full_5 metrics.Counter

	Full_6 metrics.Counter

	Full_7 metrics.Counter

	Full_8 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbminthbmdropctrlMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbminthbmdropctrlMetrics) Size() int {
	sz := 0

	sz += mtr.Full_0.Size()

	sz += mtr.Full_1.Size()

	sz += mtr.Full_2.Size()

	sz += mtr.Full_3.Size()

	sz += mtr.Full_4.Size()

	sz += mtr.Full_5.Size()

	sz += mtr.Full_6.Size()

	sz += mtr.Full_7.Size()

	sz += mtr.Full_8.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbminthbmdropctrlMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Full_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_0.Size()

	mtr.Full_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_1.Size()

	mtr.Full_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_2.Size()

	mtr.Full_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_3.Size()

	mtr.Full_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_4.Size()

	mtr.Full_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_5.Size()

	mtr.Full_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_6.Size()

	mtr.Full_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_7.Size()

	mtr.Full_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.Full_8.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbminthbmdropctrlMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Full_0" {
		return offset
	}
	offset += mtr.Full_0.Size()

	if fldName == "Full_1" {
		return offset
	}
	offset += mtr.Full_1.Size()

	if fldName == "Full_2" {
		return offset
	}
	offset += mtr.Full_2.Size()

	if fldName == "Full_3" {
		return offset
	}
	offset += mtr.Full_3.Size()

	if fldName == "Full_4" {
		return offset
	}
	offset += mtr.Full_4.Size()

	if fldName == "Full_5" {
		return offset
	}
	offset += mtr.Full_5.Size()

	if fldName == "Full_6" {
		return offset
	}
	offset += mtr.Full_6.Size()

	if fldName == "Full_7" {
		return offset
	}
	offset += mtr.Full_7.Size()

	if fldName == "Full_8" {
		return offset
	}
	offset += mtr.Full_8.Size()

	return offset
}

// SetFull_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropctrlMetrics) SetFull_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_0"))
	return nil
}

// SetFull_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropctrlMetrics) SetFull_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_1"))
	return nil
}

// SetFull_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropctrlMetrics) SetFull_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_2"))
	return nil
}

// SetFull_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropctrlMetrics) SetFull_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_3"))
	return nil
}

// SetFull_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropctrlMetrics) SetFull_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_4"))
	return nil
}

// SetFull_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropctrlMetrics) SetFull_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_5"))
	return nil
}

// SetFull_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropctrlMetrics) SetFull_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_6"))
	return nil
}

// SetFull_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropctrlMetrics) SetFull_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_7"))
	return nil
}

// SetFull_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropctrlMetrics) SetFull_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Full_8"))
	return nil
}

// PbpbchbminthbmdropctrlMetricsIterator is the iterator object
type PbpbchbminthbmdropctrlMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbminthbmdropctrlMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbminthbmdropctrlMetricsIterator) Next() *PbpbchbminthbmdropctrlMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbchbminthbmdropctrlMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbminthbmdropctrlMetricsIterator) Find(key uint64) (*PbpbchbminthbmdropctrlMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbminthbmdropctrlMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbminthbmdropctrlMetricsIterator) Create(key uint64) (*PbpbchbminthbmdropctrlMetrics, error) {
	tmtr := &PbpbchbminthbmdropctrlMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbminthbmdropctrlMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbminthbmdropctrlMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbchbminthbmdropctrlMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbchbminthbmdropctrlMetricsIterator returns an iterator
func NewPbpbchbminthbmdropctrlMetricsIterator() (*PbpbchbminthbmdropctrlMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbminthbmdropctrlMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbminthbmdropctrlMetricsIterator{iter: iter}, nil
}

type PbpbchbminthbmdropthresholdMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Crossed_0 metrics.Counter

	Crossed_1 metrics.Counter

	Crossed_2 metrics.Counter

	Crossed_3 metrics.Counter

	Crossed_4 metrics.Counter

	Crossed_5 metrics.Counter

	Crossed_6 metrics.Counter

	Crossed_7 metrics.Counter

	Crossed_8 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbminthbmdropthresholdMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbminthbmdropthresholdMetrics) Size() int {
	sz := 0

	sz += mtr.Crossed_0.Size()

	sz += mtr.Crossed_1.Size()

	sz += mtr.Crossed_2.Size()

	sz += mtr.Crossed_3.Size()

	sz += mtr.Crossed_4.Size()

	sz += mtr.Crossed_5.Size()

	sz += mtr.Crossed_6.Size()

	sz += mtr.Crossed_7.Size()

	sz += mtr.Crossed_8.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbminthbmdropthresholdMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Crossed_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Crossed_0.Size()

	mtr.Crossed_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Crossed_1.Size()

	mtr.Crossed_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Crossed_2.Size()

	mtr.Crossed_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Crossed_3.Size()

	mtr.Crossed_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.Crossed_4.Size()

	mtr.Crossed_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.Crossed_5.Size()

	mtr.Crossed_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.Crossed_6.Size()

	mtr.Crossed_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.Crossed_7.Size()

	mtr.Crossed_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.Crossed_8.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbminthbmdropthresholdMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Crossed_0" {
		return offset
	}
	offset += mtr.Crossed_0.Size()

	if fldName == "Crossed_1" {
		return offset
	}
	offset += mtr.Crossed_1.Size()

	if fldName == "Crossed_2" {
		return offset
	}
	offset += mtr.Crossed_2.Size()

	if fldName == "Crossed_3" {
		return offset
	}
	offset += mtr.Crossed_3.Size()

	if fldName == "Crossed_4" {
		return offset
	}
	offset += mtr.Crossed_4.Size()

	if fldName == "Crossed_5" {
		return offset
	}
	offset += mtr.Crossed_5.Size()

	if fldName == "Crossed_6" {
		return offset
	}
	offset += mtr.Crossed_6.Size()

	if fldName == "Crossed_7" {
		return offset
	}
	offset += mtr.Crossed_7.Size()

	if fldName == "Crossed_8" {
		return offset
	}
	offset += mtr.Crossed_8.Size()

	return offset
}

// SetCrossed_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropthresholdMetrics) SetCrossed_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Crossed_0"))
	return nil
}

// SetCrossed_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropthresholdMetrics) SetCrossed_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Crossed_1"))
	return nil
}

// SetCrossed_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropthresholdMetrics) SetCrossed_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Crossed_2"))
	return nil
}

// SetCrossed_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropthresholdMetrics) SetCrossed_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Crossed_3"))
	return nil
}

// SetCrossed_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropthresholdMetrics) SetCrossed_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Crossed_4"))
	return nil
}

// SetCrossed_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropthresholdMetrics) SetCrossed_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Crossed_5"))
	return nil
}

// SetCrossed_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropthresholdMetrics) SetCrossed_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Crossed_6"))
	return nil
}

// SetCrossed_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropthresholdMetrics) SetCrossed_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Crossed_7"))
	return nil
}

// SetCrossed_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropthresholdMetrics) SetCrossed_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Crossed_8"))
	return nil
}

// PbpbchbminthbmdropthresholdMetricsIterator is the iterator object
type PbpbchbminthbmdropthresholdMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbminthbmdropthresholdMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbminthbmdropthresholdMetricsIterator) Next() *PbpbchbminthbmdropthresholdMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbchbminthbmdropthresholdMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbminthbmdropthresholdMetricsIterator) Find(key uint64) (*PbpbchbminthbmdropthresholdMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbminthbmdropthresholdMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbminthbmdropthresholdMetricsIterator) Create(key uint64) (*PbpbchbminthbmdropthresholdMetrics, error) {
	tmtr := &PbpbchbminthbmdropthresholdMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbminthbmdropthresholdMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbminthbmdropthresholdMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbchbminthbmdropthresholdMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbchbminthbmdropthresholdMetricsIterator returns an iterator
func NewPbpbchbminthbmdropthresholdMetricsIterator() (*PbpbchbminthbmdropthresholdMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbminthbmdropthresholdMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbminthbmdropthresholdMetricsIterator{iter: iter}, nil
}

type PbpbchbminthbmtruncatenoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Drop_0 metrics.Counter

	Drop_1 metrics.Counter

	Drop_2 metrics.Counter

	Drop_3 metrics.Counter

	Drop_4 metrics.Counter

	Drop_5 metrics.Counter

	Drop_6 metrics.Counter

	Drop_7 metrics.Counter

	Drop_8 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbminthbmtruncatenoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbminthbmtruncatenoMetrics) Size() int {
	sz := 0

	sz += mtr.Drop_0.Size()

	sz += mtr.Drop_1.Size()

	sz += mtr.Drop_2.Size()

	sz += mtr.Drop_3.Size()

	sz += mtr.Drop_4.Size()

	sz += mtr.Drop_5.Size()

	sz += mtr.Drop_6.Size()

	sz += mtr.Drop_7.Size()

	sz += mtr.Drop_8.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbminthbmtruncatenoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Drop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Drop_0.Size()

	mtr.Drop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Drop_1.Size()

	mtr.Drop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Drop_2.Size()

	mtr.Drop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Drop_3.Size()

	mtr.Drop_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.Drop_4.Size()

	mtr.Drop_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.Drop_5.Size()

	mtr.Drop_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.Drop_6.Size()

	mtr.Drop_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.Drop_7.Size()

	mtr.Drop_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.Drop_8.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbminthbmtruncatenoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Drop_0" {
		return offset
	}
	offset += mtr.Drop_0.Size()

	if fldName == "Drop_1" {
		return offset
	}
	offset += mtr.Drop_1.Size()

	if fldName == "Drop_2" {
		return offset
	}
	offset += mtr.Drop_2.Size()

	if fldName == "Drop_3" {
		return offset
	}
	offset += mtr.Drop_3.Size()

	if fldName == "Drop_4" {
		return offset
	}
	offset += mtr.Drop_4.Size()

	if fldName == "Drop_5" {
		return offset
	}
	offset += mtr.Drop_5.Size()

	if fldName == "Drop_6" {
		return offset
	}
	offset += mtr.Drop_6.Size()

	if fldName == "Drop_7" {
		return offset
	}
	offset += mtr.Drop_7.Size()

	if fldName == "Drop_8" {
		return offset
	}
	offset += mtr.Drop_8.Size()

	return offset
}

// SetDrop_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmtruncatenoMetrics) SetDrop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Drop_0"))
	return nil
}

// SetDrop_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmtruncatenoMetrics) SetDrop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Drop_1"))
	return nil
}

// SetDrop_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmtruncatenoMetrics) SetDrop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Drop_2"))
	return nil
}

// SetDrop_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmtruncatenoMetrics) SetDrop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Drop_3"))
	return nil
}

// SetDrop_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmtruncatenoMetrics) SetDrop_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Drop_4"))
	return nil
}

// SetDrop_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmtruncatenoMetrics) SetDrop_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Drop_5"))
	return nil
}

// SetDrop_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmtruncatenoMetrics) SetDrop_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Drop_6"))
	return nil
}

// SetDrop_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmtruncatenoMetrics) SetDrop_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Drop_7"))
	return nil
}

// SetDrop_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmtruncatenoMetrics) SetDrop_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Drop_8"))
	return nil
}

// PbpbchbminthbmtruncatenoMetricsIterator is the iterator object
type PbpbchbminthbmtruncatenoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbminthbmtruncatenoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbminthbmtruncatenoMetricsIterator) Next() *PbpbchbminthbmtruncatenoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbchbminthbmtruncatenoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbminthbmtruncatenoMetricsIterator) Find(key uint64) (*PbpbchbminthbmtruncatenoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbminthbmtruncatenoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbminthbmtruncatenoMetricsIterator) Create(key uint64) (*PbpbchbminthbmtruncatenoMetrics, error) {
	tmtr := &PbpbchbminthbmtruncatenoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbminthbmtruncatenoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbminthbmtruncatenoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbchbminthbmtruncatenoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbchbminthbmtruncatenoMetricsIterator returns an iterator
func NewPbpbchbminthbmtruncatenoMetricsIterator() (*PbpbchbminthbmtruncatenoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbminthbmtruncatenoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbminthbmtruncatenoMetricsIterator{iter: iter}, nil
}

type Pmpbmintecccol0Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pmpbmintecccol0Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pmpbmintecccol0Metrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pmpbmintecccol0Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pmpbmintecccol0Metrics) getOffset(fldName string) int {
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
func (mtr *Pmpbmintecccol0Metrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *Pmpbmintecccol0Metrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// Pmpbmintecccol0MetricsIterator is the iterator object
type Pmpbmintecccol0MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pmpbmintecccol0MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pmpbmintecccol0MetricsIterator) Next() *Pmpbmintecccol0Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pmpbmintecccol0Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pmpbmintecccol0MetricsIterator) Find(key uint64) (*Pmpbmintecccol0Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pmpbmintecccol0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pmpbmintecccol0MetricsIterator) Create(key uint64) (*Pmpbmintecccol0Metrics, error) {
	tmtr := &Pmpbmintecccol0Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pmpbmintecccol0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pmpbmintecccol0MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pmpbmintecccol0MetricsIterator) Free() {
	it.iter.Free()
}

// NewPmpbmintecccol0MetricsIterator returns an iterator
func NewPmpbmintecccol0MetricsIterator() (*Pmpbmintecccol0MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pmpbmintecccol0Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pmpbmintecccol0MetricsIterator{iter: iter}, nil
}

type Pmpbmintecccol1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pmpbmintecccol1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pmpbmintecccol1Metrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pmpbmintecccol1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pmpbmintecccol1Metrics) getOffset(fldName string) int {
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
func (mtr *Pmpbmintecccol1Metrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *Pmpbmintecccol1Metrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// Pmpbmintecccol1MetricsIterator is the iterator object
type Pmpbmintecccol1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pmpbmintecccol1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pmpbmintecccol1MetricsIterator) Next() *Pmpbmintecccol1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pmpbmintecccol1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pmpbmintecccol1MetricsIterator) Find(key uint64) (*Pmpbmintecccol1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pmpbmintecccol1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pmpbmintecccol1MetricsIterator) Create(key uint64) (*Pmpbmintecccol1Metrics, error) {
	tmtr := &Pmpbmintecccol1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pmpbmintecccol1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pmpbmintecccol1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pmpbmintecccol1MetricsIterator) Free() {
	it.iter.Free()
}

// NewPmpbmintecccol1MetricsIterator returns an iterator
func NewPmpbmintecccol1MetricsIterator() (*Pmpbmintecccol1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pmpbmintecccol1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pmpbmintecccol1MetricsIterator{iter: iter}, nil
}

type Pmpbmintecccol2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pmpbmintecccol2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pmpbmintecccol2Metrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pmpbmintecccol2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pmpbmintecccol2Metrics) getOffset(fldName string) int {
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
func (mtr *Pmpbmintecccol2Metrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *Pmpbmintecccol2Metrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// Pmpbmintecccol2MetricsIterator is the iterator object
type Pmpbmintecccol2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pmpbmintecccol2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pmpbmintecccol2MetricsIterator) Next() *Pmpbmintecccol2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pmpbmintecccol2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pmpbmintecccol2MetricsIterator) Find(key uint64) (*Pmpbmintecccol2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pmpbmintecccol2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pmpbmintecccol2MetricsIterator) Create(key uint64) (*Pmpbmintecccol2Metrics, error) {
	tmtr := &Pmpbmintecccol2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pmpbmintecccol2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pmpbmintecccol2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pmpbmintecccol2MetricsIterator) Free() {
	it.iter.Free()
}

// NewPmpbmintecccol2MetricsIterator returns an iterator
func NewPmpbmintecccol2MetricsIterator() (*Pmpbmintecccol2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pmpbmintecccol2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pmpbmintecccol2MetricsIterator{iter: iter}, nil
}

type Pmpbmintecccol3Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pmpbmintecccol3Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pmpbmintecccol3Metrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pmpbmintecccol3Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pmpbmintecccol3Metrics) getOffset(fldName string) int {
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
func (mtr *Pmpbmintecccol3Metrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *Pmpbmintecccol3Metrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// Pmpbmintecccol3MetricsIterator is the iterator object
type Pmpbmintecccol3MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pmpbmintecccol3MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pmpbmintecccol3MetricsIterator) Next() *Pmpbmintecccol3Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pmpbmintecccol3Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pmpbmintecccol3MetricsIterator) Find(key uint64) (*Pmpbmintecccol3Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pmpbmintecccol3Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pmpbmintecccol3MetricsIterator) Create(key uint64) (*Pmpbmintecccol3Metrics, error) {
	tmtr := &Pmpbmintecccol3Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pmpbmintecccol3Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pmpbmintecccol3MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pmpbmintecccol3MetricsIterator) Free() {
	it.iter.Free()
}

// NewPmpbmintecccol3MetricsIterator returns an iterator
func NewPmpbmintecccol3MetricsIterator() (*Pmpbmintecccol3MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pmpbmintecccol3Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pmpbmintecccol3MetricsIterator{iter: iter}, nil
}

type Pmpbmintecccol4Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pmpbmintecccol4Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pmpbmintecccol4Metrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pmpbmintecccol4Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pmpbmintecccol4Metrics) getOffset(fldName string) int {
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
func (mtr *Pmpbmintecccol4Metrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *Pmpbmintecccol4Metrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// Pmpbmintecccol4MetricsIterator is the iterator object
type Pmpbmintecccol4MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pmpbmintecccol4MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pmpbmintecccol4MetricsIterator) Next() *Pmpbmintecccol4Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pmpbmintecccol4Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pmpbmintecccol4MetricsIterator) Find(key uint64) (*Pmpbmintecccol4Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pmpbmintecccol4Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pmpbmintecccol4MetricsIterator) Create(key uint64) (*Pmpbmintecccol4Metrics, error) {
	tmtr := &Pmpbmintecccol4Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pmpbmintecccol4Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pmpbmintecccol4MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pmpbmintecccol4MetricsIterator) Free() {
	it.iter.Free()
}

// NewPmpbmintecccol4MetricsIterator returns an iterator
func NewPmpbmintecccol4MetricsIterator() (*Pmpbmintecccol4MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pmpbmintecccol4Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pmpbmintecccol4MetricsIterator{iter: iter}, nil
}

type DbwaintlifqstatemapMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	EccUncorrectable metrics.Counter

	EccCorrectable metrics.Counter

	QidInvalid metrics.Counter

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
func (mtr *DbwaintlifqstatemapMetrics) SetEccUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccUncorrectable"))
	return nil
}

// SetEccCorrectable sets cunter in shared memory
func (mtr *DbwaintlifqstatemapMetrics) SetEccCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccCorrectable"))
	return nil
}

// SetQidInvalid sets cunter in shared memory
func (mtr *DbwaintlifqstatemapMetrics) SetQidInvalid(val metrics.Counter) error {
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
	if mtr == nil {
		return nil
	}

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

// Free frees the iterator memory
func (it *DbwaintlifqstatemapMetricsIterator) Free() {
	it.iter.Free()
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

type DbwaintdbMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DbCamConflict metrics.Counter

	DbPidChkFail metrics.Counter

	DbQidOverflow metrics.Counter

	HostRingAccessErr metrics.Counter

	TotalRingAccessErr metrics.Counter

	RrespErr metrics.Counter

	BrespErr metrics.Counter

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
func (mtr *DbwaintdbMetrics) SetDbCamConflict(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DbCamConflict"))
	return nil
}

// SetDbPidChkFail sets cunter in shared memory
func (mtr *DbwaintdbMetrics) SetDbPidChkFail(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DbPidChkFail"))
	return nil
}

// SetDbQidOverflow sets cunter in shared memory
func (mtr *DbwaintdbMetrics) SetDbQidOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DbQidOverflow"))
	return nil
}

// SetHostRingAccessErr sets cunter in shared memory
func (mtr *DbwaintdbMetrics) SetHostRingAccessErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("HostRingAccessErr"))
	return nil
}

// SetTotalRingAccessErr sets cunter in shared memory
func (mtr *DbwaintdbMetrics) SetTotalRingAccessErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TotalRingAccessErr"))
	return nil
}

// SetRrespErr sets cunter in shared memory
func (mtr *DbwaintdbMetrics) SetRrespErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RrespErr"))
	return nil
}

// SetBrespErr sets cunter in shared memory
func (mtr *DbwaintdbMetrics) SetBrespErr(val metrics.Counter) error {
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
	if mtr == nil {
		return nil
	}

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

// Free frees the iterator memory
func (it *DbwaintdbMetricsIterator) Free() {
	it.iter.Free()
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

type SsipicsintpicsMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	UncorrectableEcc metrics.Counter

	CorrectableEcc metrics.Counter

	TooManyRlSchError metrics.Counter

	Picc metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *SsipicsintpicsMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *SsipicsintpicsMetrics) Size() int {
	sz := 0

	sz += mtr.UncorrectableEcc.Size()

	sz += mtr.CorrectableEcc.Size()

	sz += mtr.TooManyRlSchError.Size()

	sz += mtr.Picc.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *SsipicsintpicsMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.UncorrectableEcc = mtr.metrics.GetCounter(offset)
	offset += mtr.UncorrectableEcc.Size()

	mtr.CorrectableEcc = mtr.metrics.GetCounter(offset)
	offset += mtr.CorrectableEcc.Size()

	mtr.TooManyRlSchError = mtr.metrics.GetCounter(offset)
	offset += mtr.TooManyRlSchError.Size()

	mtr.Picc = mtr.metrics.GetCounter(offset)
	offset += mtr.Picc.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *SsipicsintpicsMetrics) getOffset(fldName string) int {
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

	if fldName == "Picc" {
		return offset
	}
	offset += mtr.Picc.Size()

	return offset
}

// SetUncorrectableEcc sets cunter in shared memory
func (mtr *SsipicsintpicsMetrics) SetUncorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UncorrectableEcc"))
	return nil
}

// SetCorrectableEcc sets cunter in shared memory
func (mtr *SsipicsintpicsMetrics) SetCorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CorrectableEcc"))
	return nil
}

// SetTooManyRlSchError sets cunter in shared memory
func (mtr *SsipicsintpicsMetrics) SetTooManyRlSchError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TooManyRlSchError"))
	return nil
}

// SetPicc sets cunter in shared memory
func (mtr *SsipicsintpicsMetrics) SetPicc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Picc"))
	return nil
}

// SsipicsintpicsMetricsIterator is the iterator object
type SsipicsintpicsMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SsipicsintpicsMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SsipicsintpicsMetricsIterator) Next() *SsipicsintpicsMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &SsipicsintpicsMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SsipicsintpicsMetricsIterator) Find(key uint64) (*SsipicsintpicsMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &SsipicsintpicsMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SsipicsintpicsMetricsIterator) Create(key uint64) (*SsipicsintpicsMetrics, error) {
	tmtr := &SsipicsintpicsMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &SsipicsintpicsMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SsipicsintpicsMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *SsipicsintpicsMetricsIterator) Free() {
	it.iter.Free()
}

// NewSsipicsintpicsMetricsIterator returns an iterator
func NewSsipicsintpicsMetricsIterator() (*SsipicsintpicsMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SsipicsintpicsMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SsipicsintpicsMetricsIterator{iter: iter}, nil
}

type SsipicsintbadaddrMetrics struct {
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

func (mtr *SsipicsintbadaddrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *SsipicsintbadaddrMetrics) Size() int {
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
func (mtr *SsipicsintbadaddrMetrics) Unmarshal() error {
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
func (mtr *SsipicsintbadaddrMetrics) getOffset(fldName string) int {
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
func (mtr *SsipicsintbadaddrMetrics) SetRdreq0BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq0BadAddr"))
	return nil
}

// SetWrreq0BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetWrreq0BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq0BadAddr"))
	return nil
}

// SetRdreq1BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetRdreq1BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq1BadAddr"))
	return nil
}

// SetWrreq1BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetWrreq1BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq1BadAddr"))
	return nil
}

// SetRdreq2BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetRdreq2BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq2BadAddr"))
	return nil
}

// SetWrreq2BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetWrreq2BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq2BadAddr"))
	return nil
}

// SetRdreq3BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetRdreq3BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq3BadAddr"))
	return nil
}

// SetWrreq3BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetWrreq3BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq3BadAddr"))
	return nil
}

// SetRdreq4BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetRdreq4BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq4BadAddr"))
	return nil
}

// SetWrreq4BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetWrreq4BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq4BadAddr"))
	return nil
}

// SetRdreq5BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetRdreq5BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq5BadAddr"))
	return nil
}

// SetWrreq5BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetWrreq5BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq5BadAddr"))
	return nil
}

// SetRdreq6BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetRdreq6BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq6BadAddr"))
	return nil
}

// SetWrreq6BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetWrreq6BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq6BadAddr"))
	return nil
}

// SetRdreq7BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetRdreq7BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq7BadAddr"))
	return nil
}

// SetWrreq7BadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetWrreq7BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq7BadAddr"))
	return nil
}

// SetCpuBadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetCpuBadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CpuBadAddr"))
	return nil
}

// SetBgBadAddr sets cunter in shared memory
func (mtr *SsipicsintbadaddrMetrics) SetBgBadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BgBadAddr"))
	return nil
}

// SsipicsintbadaddrMetricsIterator is the iterator object
type SsipicsintbadaddrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SsipicsintbadaddrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SsipicsintbadaddrMetricsIterator) Next() *SsipicsintbadaddrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &SsipicsintbadaddrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SsipicsintbadaddrMetricsIterator) Find(key uint64) (*SsipicsintbadaddrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &SsipicsintbadaddrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SsipicsintbadaddrMetricsIterator) Create(key uint64) (*SsipicsintbadaddrMetrics, error) {
	tmtr := &SsipicsintbadaddrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &SsipicsintbadaddrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SsipicsintbadaddrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *SsipicsintbadaddrMetricsIterator) Free() {
	it.iter.Free()
}

// NewSsipicsintbadaddrMetricsIterator returns an iterator
func NewSsipicsintbadaddrMetricsIterator() (*SsipicsintbadaddrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SsipicsintbadaddrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SsipicsintbadaddrMetricsIterator{iter: iter}, nil
}

type SsipicsintbgMetrics struct {
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

func (mtr *SsipicsintbgMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *SsipicsintbgMetrics) Size() int {
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
func (mtr *SsipicsintbgMetrics) Unmarshal() error {
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
func (mtr *SsipicsintbgMetrics) getOffset(fldName string) int {
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
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg0"))
	return nil
}

// SetUnfinishedBg1 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg1"))
	return nil
}

// SetUnfinishedBg2 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg2"))
	return nil
}

// SetUnfinishedBg3 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg3"))
	return nil
}

// SetUnfinishedBg4 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg4"))
	return nil
}

// SetUnfinishedBg5 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg5"))
	return nil
}

// SetUnfinishedBg6 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg6"))
	return nil
}

// SetUnfinishedBg7 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg7"))
	return nil
}

// SetUnfinishedBg8 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg8"))
	return nil
}

// SetUnfinishedBg9 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg9"))
	return nil
}

// SetUnfinishedBg10 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg10"))
	return nil
}

// SetUnfinishedBg11 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg11"))
	return nil
}

// SetUnfinishedBg12 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg12(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg12"))
	return nil
}

// SetUnfinishedBg13 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg13(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg13"))
	return nil
}

// SetUnfinishedBg14 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg14(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg14"))
	return nil
}

// SetUnfinishedBg15 sets cunter in shared memory
func (mtr *SsipicsintbgMetrics) SetUnfinishedBg15(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg15"))
	return nil
}

// SsipicsintbgMetricsIterator is the iterator object
type SsipicsintbgMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SsipicsintbgMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SsipicsintbgMetricsIterator) Next() *SsipicsintbgMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &SsipicsintbgMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SsipicsintbgMetricsIterator) Find(key uint64) (*SsipicsintbgMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &SsipicsintbgMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SsipicsintbgMetricsIterator) Create(key uint64) (*SsipicsintbgMetrics, error) {
	tmtr := &SsipicsintbgMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &SsipicsintbgMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SsipicsintbgMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *SsipicsintbgMetricsIterator) Free() {
	it.iter.Free()
}

// NewSsipicsintbgMetricsIterator returns an iterator
func NewSsipicsintbgMetricsIterator() (*SsipicsintbgMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SsipicsintbgMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SsipicsintbgMetricsIterator{iter: iter}, nil
}

type SsipicspiccintpiccMetrics struct {
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

func (mtr *SsipicspiccintpiccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *SsipicspiccintpiccMetrics) Size() int {
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
func (mtr *SsipicspiccintpiccMetrics) Unmarshal() error {
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
func (mtr *SsipicspiccintpiccMetrics) getOffset(fldName string) int {
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
func (mtr *SsipicspiccintpiccMetrics) SetUncorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UncorrectableEcc"))
	return nil
}

// SetCorrectableEcc sets cunter in shared memory
func (mtr *SsipicspiccintpiccMetrics) SetCorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CorrectableEcc"))
	return nil
}

// SetCacheStgAwlenErr sets cunter in shared memory
func (mtr *SsipicspiccintpiccMetrics) SetCacheStgAwlenErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheStgAwlenErr"))
	return nil
}

// SetCacheHbmRrespSlverr sets cunter in shared memory
func (mtr *SsipicspiccintpiccMetrics) SetCacheHbmRrespSlverr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmRrespSlverr"))
	return nil
}

// SetCacheHbmRrespDecerr sets cunter in shared memory
func (mtr *SsipicspiccintpiccMetrics) SetCacheHbmRrespDecerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmRrespDecerr"))
	return nil
}

// SetCacheHbmBrespSlverr sets cunter in shared memory
func (mtr *SsipicspiccintpiccMetrics) SetCacheHbmBrespSlverr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmBrespSlverr"))
	return nil
}

// SetCacheHbmBrespDecerr sets cunter in shared memory
func (mtr *SsipicspiccintpiccMetrics) SetCacheHbmBrespDecerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmBrespDecerr"))
	return nil
}

// SsipicspiccintpiccMetricsIterator is the iterator object
type SsipicspiccintpiccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SsipicspiccintpiccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SsipicspiccintpiccMetricsIterator) Next() *SsipicspiccintpiccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &SsipicspiccintpiccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SsipicspiccintpiccMetricsIterator) Find(key uint64) (*SsipicspiccintpiccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &SsipicspiccintpiccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SsipicspiccintpiccMetricsIterator) Create(key uint64) (*SsipicspiccintpiccMetrics, error) {
	tmtr := &SsipicspiccintpiccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &SsipicspiccintpiccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SsipicspiccintpiccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *SsipicspiccintpiccMetricsIterator) Free() {
	it.iter.Free()
}

// NewSsipicspiccintpiccMetricsIterator returns an iterator
func NewSsipicspiccintpiccMetricsIterator() (*SsipicspiccintpiccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SsipicspiccintpiccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SsipicspiccintpiccMetricsIterator{iter: iter}, nil
}

type SsepicsintpicsMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	UncorrectableEcc metrics.Counter

	CorrectableEcc metrics.Counter

	TooManyRlSchError metrics.Counter

	Picc metrics.Counter

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

	sz += mtr.Picc.Size()

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

	mtr.Picc = mtr.metrics.GetCounter(offset)
	offset += mtr.Picc.Size()

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

	if fldName == "Picc" {
		return offset
	}
	offset += mtr.Picc.Size()

	return offset
}

// SetUncorrectableEcc sets cunter in shared memory
func (mtr *SsepicsintpicsMetrics) SetUncorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UncorrectableEcc"))
	return nil
}

// SetCorrectableEcc sets cunter in shared memory
func (mtr *SsepicsintpicsMetrics) SetCorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CorrectableEcc"))
	return nil
}

// SetTooManyRlSchError sets cunter in shared memory
func (mtr *SsepicsintpicsMetrics) SetTooManyRlSchError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TooManyRlSchError"))
	return nil
}

// SetPicc sets cunter in shared memory
func (mtr *SsepicsintpicsMetrics) SetPicc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Picc"))
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
	if mtr == nil {
		return nil
	}

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

// Free frees the iterator memory
func (it *SsepicsintpicsMetricsIterator) Free() {
	it.iter.Free()
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

type SsepicsintbadaddrMetrics struct {
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
func (mtr *SsepicsintbadaddrMetrics) SetRdreq0BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq0BadAddr"))
	return nil
}

// SetWrreq0BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq0BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq0BadAddr"))
	return nil
}

// SetRdreq1BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq1BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq1BadAddr"))
	return nil
}

// SetWrreq1BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq1BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq1BadAddr"))
	return nil
}

// SetRdreq2BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq2BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq2BadAddr"))
	return nil
}

// SetWrreq2BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq2BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq2BadAddr"))
	return nil
}

// SetRdreq3BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq3BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq3BadAddr"))
	return nil
}

// SetWrreq3BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq3BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq3BadAddr"))
	return nil
}

// SetRdreq4BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq4BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq4BadAddr"))
	return nil
}

// SetWrreq4BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq4BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq4BadAddr"))
	return nil
}

// SetRdreq5BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq5BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq5BadAddr"))
	return nil
}

// SetWrreq5BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq5BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq5BadAddr"))
	return nil
}

// SetRdreq6BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq6BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq6BadAddr"))
	return nil
}

// SetWrreq6BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq6BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq6BadAddr"))
	return nil
}

// SetRdreq7BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq7BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq7BadAddr"))
	return nil
}

// SetWrreq7BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq7BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq7BadAddr"))
	return nil
}

// SetCpuBadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetCpuBadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CpuBadAddr"))
	return nil
}

// SetBgBadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetBgBadAddr(val metrics.Counter) error {
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
	if mtr == nil {
		return nil
	}

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

// Free frees the iterator memory
func (it *SsepicsintbadaddrMetricsIterator) Free() {
	it.iter.Free()
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
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg0"))
	return nil
}

// SetUnfinishedBg1 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg1"))
	return nil
}

// SetUnfinishedBg2 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg2"))
	return nil
}

// SetUnfinishedBg3 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg3"))
	return nil
}

// SetUnfinishedBg4 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg4"))
	return nil
}

// SetUnfinishedBg5 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg5"))
	return nil
}

// SetUnfinishedBg6 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg6"))
	return nil
}

// SetUnfinishedBg7 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg7"))
	return nil
}

// SetUnfinishedBg8 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg8"))
	return nil
}

// SetUnfinishedBg9 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg9"))
	return nil
}

// SetUnfinishedBg10 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg10"))
	return nil
}

// SetUnfinishedBg11 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg11"))
	return nil
}

// SetUnfinishedBg12 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg12(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg12"))
	return nil
}

// SetUnfinishedBg13 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg13(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg13"))
	return nil
}

// SetUnfinishedBg14 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg14(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg14"))
	return nil
}

// SetUnfinishedBg15 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg15(val metrics.Counter) error {
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
	if mtr == nil {
		return nil
	}

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

// Free frees the iterator memory
func (it *SsepicsintbgMetricsIterator) Free() {
	it.iter.Free()
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

type SsepicspiccintpiccMetrics struct {
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

func (mtr *SsepicspiccintpiccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *SsepicspiccintpiccMetrics) Size() int {
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
func (mtr *SsepicspiccintpiccMetrics) Unmarshal() error {
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
func (mtr *SsepicspiccintpiccMetrics) getOffset(fldName string) int {
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
func (mtr *SsepicspiccintpiccMetrics) SetUncorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UncorrectableEcc"))
	return nil
}

// SetCorrectableEcc sets cunter in shared memory
func (mtr *SsepicspiccintpiccMetrics) SetCorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CorrectableEcc"))
	return nil
}

// SetCacheStgAwlenErr sets cunter in shared memory
func (mtr *SsepicspiccintpiccMetrics) SetCacheStgAwlenErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheStgAwlenErr"))
	return nil
}

// SetCacheHbmRrespSlverr sets cunter in shared memory
func (mtr *SsepicspiccintpiccMetrics) SetCacheHbmRrespSlverr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmRrespSlverr"))
	return nil
}

// SetCacheHbmRrespDecerr sets cunter in shared memory
func (mtr *SsepicspiccintpiccMetrics) SetCacheHbmRrespDecerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmRrespDecerr"))
	return nil
}

// SetCacheHbmBrespSlverr sets cunter in shared memory
func (mtr *SsepicspiccintpiccMetrics) SetCacheHbmBrespSlverr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmBrespSlverr"))
	return nil
}

// SetCacheHbmBrespDecerr sets cunter in shared memory
func (mtr *SsepicspiccintpiccMetrics) SetCacheHbmBrespDecerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmBrespDecerr"))
	return nil
}

// SsepicspiccintpiccMetricsIterator is the iterator object
type SsepicspiccintpiccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SsepicspiccintpiccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SsepicspiccintpiccMetricsIterator) Next() *SsepicspiccintpiccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &SsepicspiccintpiccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SsepicspiccintpiccMetricsIterator) Find(key uint64) (*SsepicspiccintpiccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &SsepicspiccintpiccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SsepicspiccintpiccMetricsIterator) Create(key uint64) (*SsepicspiccintpiccMetrics, error) {
	tmtr := &SsepicspiccintpiccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &SsepicspiccintpiccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SsepicspiccintpiccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *SsepicspiccintpiccMetricsIterator) Free() {
	it.iter.Free()
}

// NewSsepicspiccintpiccMetricsIterator returns an iterator
func NewSsepicspiccintpiccMetricsIterator() (*SsepicspiccintpiccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SsepicspiccintpiccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SsepicspiccintpiccMetricsIterator{iter: iter}, nil
}

type BxbxintmacMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	LaneSbe metrics.Counter

	LaneDbe metrics.Counter

	Xdmac10Intr metrics.Counter

	Xdmac10Pslverr metrics.Counter

	BxpbPbusDrdy metrics.Counter

	RxMissingSof metrics.Counter

	RxMissingEof metrics.Counter

	RxTimeoutErr metrics.Counter

	RxMinSizeErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *BxbxintmacMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *BxbxintmacMetrics) Size() int {
	sz := 0

	sz += mtr.LaneSbe.Size()

	sz += mtr.LaneDbe.Size()

	sz += mtr.Xdmac10Intr.Size()

	sz += mtr.Xdmac10Pslverr.Size()

	sz += mtr.BxpbPbusDrdy.Size()

	sz += mtr.RxMissingSof.Size()

	sz += mtr.RxMissingEof.Size()

	sz += mtr.RxTimeoutErr.Size()

	sz += mtr.RxMinSizeErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *BxbxintmacMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.LaneSbe = mtr.metrics.GetCounter(offset)
	offset += mtr.LaneSbe.Size()

	mtr.LaneDbe = mtr.metrics.GetCounter(offset)
	offset += mtr.LaneDbe.Size()

	mtr.Xdmac10Intr = mtr.metrics.GetCounter(offset)
	offset += mtr.Xdmac10Intr.Size()

	mtr.Xdmac10Pslverr = mtr.metrics.GetCounter(offset)
	offset += mtr.Xdmac10Pslverr.Size()

	mtr.BxpbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.BxpbPbusDrdy.Size()

	mtr.RxMissingSof = mtr.metrics.GetCounter(offset)
	offset += mtr.RxMissingSof.Size()

	mtr.RxMissingEof = mtr.metrics.GetCounter(offset)
	offset += mtr.RxMissingEof.Size()

	mtr.RxTimeoutErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RxTimeoutErr.Size()

	mtr.RxMinSizeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RxMinSizeErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *BxbxintmacMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "LaneSbe" {
		return offset
	}
	offset += mtr.LaneSbe.Size()

	if fldName == "LaneDbe" {
		return offset
	}
	offset += mtr.LaneDbe.Size()

	if fldName == "Xdmac10Intr" {
		return offset
	}
	offset += mtr.Xdmac10Intr.Size()

	if fldName == "Xdmac10Pslverr" {
		return offset
	}
	offset += mtr.Xdmac10Pslverr.Size()

	if fldName == "BxpbPbusDrdy" {
		return offset
	}
	offset += mtr.BxpbPbusDrdy.Size()

	if fldName == "RxMissingSof" {
		return offset
	}
	offset += mtr.RxMissingSof.Size()

	if fldName == "RxMissingEof" {
		return offset
	}
	offset += mtr.RxMissingEof.Size()

	if fldName == "RxTimeoutErr" {
		return offset
	}
	offset += mtr.RxTimeoutErr.Size()

	if fldName == "RxMinSizeErr" {
		return offset
	}
	offset += mtr.RxMinSizeErr.Size()

	return offset
}

// SetLaneSbe sets cunter in shared memory
func (mtr *BxbxintmacMetrics) SetLaneSbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LaneSbe"))
	return nil
}

// SetLaneDbe sets cunter in shared memory
func (mtr *BxbxintmacMetrics) SetLaneDbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LaneDbe"))
	return nil
}

// SetXdmac10Intr sets cunter in shared memory
func (mtr *BxbxintmacMetrics) SetXdmac10Intr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Xdmac10Intr"))
	return nil
}

// SetXdmac10Pslverr sets cunter in shared memory
func (mtr *BxbxintmacMetrics) SetXdmac10Pslverr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Xdmac10Pslverr"))
	return nil
}

// SetBxpbPbusDrdy sets cunter in shared memory
func (mtr *BxbxintmacMetrics) SetBxpbPbusDrdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BxpbPbusDrdy"))
	return nil
}

// SetRxMissingSof sets cunter in shared memory
func (mtr *BxbxintmacMetrics) SetRxMissingSof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxMissingSof"))
	return nil
}

// SetRxMissingEof sets cunter in shared memory
func (mtr *BxbxintmacMetrics) SetRxMissingEof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxMissingEof"))
	return nil
}

// SetRxTimeoutErr sets cunter in shared memory
func (mtr *BxbxintmacMetrics) SetRxTimeoutErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxTimeoutErr"))
	return nil
}

// SetRxMinSizeErr sets cunter in shared memory
func (mtr *BxbxintmacMetrics) SetRxMinSizeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxMinSizeErr"))
	return nil
}

// BxbxintmacMetricsIterator is the iterator object
type BxbxintmacMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *BxbxintmacMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *BxbxintmacMetricsIterator) Next() *BxbxintmacMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &BxbxintmacMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *BxbxintmacMetricsIterator) Find(key uint64) (*BxbxintmacMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &BxbxintmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *BxbxintmacMetricsIterator) Create(key uint64) (*BxbxintmacMetrics, error) {
	tmtr := &BxbxintmacMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &BxbxintmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *BxbxintmacMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *BxbxintmacMetricsIterator) Free() {
	it.iter.Free()
}

// NewBxbxintmacMetricsIterator returns an iterator
func NewBxbxintmacMetricsIterator() (*BxbxintmacMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("BxbxintmacMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &BxbxintmacMetricsIterator{iter: iter}, nil
}

type MdhensintipcoreMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	XtsEnc metrics.Counter

	Xts metrics.Counter

	Gcm0 metrics.Counter

	Gcm1 metrics.Counter

	Drbg metrics.Counter

	Pk metrics.Counter

	Cp metrics.Counter

	Dc metrics.Counter

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
func (mtr *MdhensintipcoreMetrics) SetXtsEnc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XtsEnc"))
	return nil
}

// SetXts sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetXts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Xts"))
	return nil
}

// SetGcm0 sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetGcm0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Gcm0"))
	return nil
}

// SetGcm1 sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetGcm1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Gcm1"))
	return nil
}

// SetDrbg sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetDrbg(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Drbg"))
	return nil
}

// SetPk sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetPk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pk"))
	return nil
}

// SetCp sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetCp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Cp"))
	return nil
}

// SetDc sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetDc(val metrics.Counter) error {
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
	if mtr == nil {
		return nil
	}

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

// Free frees the iterator memory
func (it *MdhensintipcoreMetricsIterator) Free() {
	it.iter.Free()
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

type MdhensintpkeccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CorrectableErr metrics.Counter

	UncorrectableErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *MdhensintpkeccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *MdhensintpkeccMetrics) Size() int {
	sz := 0

	sz += mtr.CorrectableErr.Size()

	sz += mtr.UncorrectableErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *MdhensintpkeccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CorrectableErr = mtr.metrics.GetCounter(offset)
	offset += mtr.CorrectableErr.Size()

	mtr.UncorrectableErr = mtr.metrics.GetCounter(offset)
	offset += mtr.UncorrectableErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *MdhensintpkeccMetrics) getOffset(fldName string) int {
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
func (mtr *MdhensintpkeccMetrics) SetCorrectableErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CorrectableErr"))
	return nil
}

// SetUncorrectableErr sets cunter in shared memory
func (mtr *MdhensintpkeccMetrics) SetUncorrectableErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UncorrectableErr"))
	return nil
}

// MdhensintpkeccMetricsIterator is the iterator object
type MdhensintpkeccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *MdhensintpkeccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *MdhensintpkeccMetricsIterator) Next() *MdhensintpkeccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &MdhensintpkeccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *MdhensintpkeccMetricsIterator) Find(key uint64) (*MdhensintpkeccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &MdhensintpkeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *MdhensintpkeccMetricsIterator) Create(key uint64) (*MdhensintpkeccMetrics, error) {
	tmtr := &MdhensintpkeccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &MdhensintpkeccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *MdhensintpkeccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *MdhensintpkeccMetricsIterator) Free() {
	it.iter.Free()
}

// NewMdhensintpkeccMetricsIterator returns an iterator
func NewMdhensintpkeccMetricsIterator() (*MdhensintpkeccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("MdhensintpkeccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &MdhensintpkeccMetricsIterator{iter: iter}, nil
}

type MdhensintdrbgintrameccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CorrectableErr metrics.Counter

	UncorrectableErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *MdhensintdrbgintrameccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *MdhensintdrbgintrameccMetrics) Size() int {
	sz := 0

	sz += mtr.CorrectableErr.Size()

	sz += mtr.UncorrectableErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *MdhensintdrbgintrameccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CorrectableErr = mtr.metrics.GetCounter(offset)
	offset += mtr.CorrectableErr.Size()

	mtr.UncorrectableErr = mtr.metrics.GetCounter(offset)
	offset += mtr.UncorrectableErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *MdhensintdrbgintrameccMetrics) getOffset(fldName string) int {
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
func (mtr *MdhensintdrbgintrameccMetrics) SetCorrectableErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CorrectableErr"))
	return nil
}

// SetUncorrectableErr sets cunter in shared memory
func (mtr *MdhensintdrbgintrameccMetrics) SetUncorrectableErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UncorrectableErr"))
	return nil
}

// MdhensintdrbgintrameccMetricsIterator is the iterator object
type MdhensintdrbgintrameccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *MdhensintdrbgintrameccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *MdhensintdrbgintrameccMetricsIterator) Next() *MdhensintdrbgintrameccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &MdhensintdrbgintrameccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *MdhensintdrbgintrameccMetricsIterator) Find(key uint64) (*MdhensintdrbgintrameccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &MdhensintdrbgintrameccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *MdhensintdrbgintrameccMetricsIterator) Create(key uint64) (*MdhensintdrbgintrameccMetrics, error) {
	tmtr := &MdhensintdrbgintrameccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &MdhensintdrbgintrameccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *MdhensintdrbgintrameccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *MdhensintdrbgintrameccMetricsIterator) Free() {
	it.iter.Free()
}

// NewMdhensintdrbgintrameccMetricsIterator returns an iterator
func NewMdhensintdrbgintrameccMetricsIterator() (*MdhensintdrbgintrameccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("MdhensintdrbgintrameccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &MdhensintdrbgintrameccMetricsIterator{iter: iter}, nil
}

type MdhensintdrbgcryptorameccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CorrectableErr metrics.Counter

	UncorrectableErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *MdhensintdrbgcryptorameccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *MdhensintdrbgcryptorameccMetrics) Size() int {
	sz := 0

	sz += mtr.CorrectableErr.Size()

	sz += mtr.UncorrectableErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *MdhensintdrbgcryptorameccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CorrectableErr = mtr.metrics.GetCounter(offset)
	offset += mtr.CorrectableErr.Size()

	mtr.UncorrectableErr = mtr.metrics.GetCounter(offset)
	offset += mtr.UncorrectableErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *MdhensintdrbgcryptorameccMetrics) getOffset(fldName string) int {
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
func (mtr *MdhensintdrbgcryptorameccMetrics) SetCorrectableErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CorrectableErr"))
	return nil
}

// SetUncorrectableErr sets cunter in shared memory
func (mtr *MdhensintdrbgcryptorameccMetrics) SetUncorrectableErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UncorrectableErr"))
	return nil
}

// MdhensintdrbgcryptorameccMetricsIterator is the iterator object
type MdhensintdrbgcryptorameccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *MdhensintdrbgcryptorameccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *MdhensintdrbgcryptorameccMetricsIterator) Next() *MdhensintdrbgcryptorameccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &MdhensintdrbgcryptorameccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *MdhensintdrbgcryptorameccMetricsIterator) Find(key uint64) (*MdhensintdrbgcryptorameccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &MdhensintdrbgcryptorameccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *MdhensintdrbgcryptorameccMetricsIterator) Create(key uint64) (*MdhensintdrbgcryptorameccMetrics, error) {
	tmtr := &MdhensintdrbgcryptorameccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &MdhensintdrbgcryptorameccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *MdhensintdrbgcryptorameccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *MdhensintdrbgcryptorameccMetricsIterator) Free() {
	it.iter.Free()
}

// NewMdhensintdrbgcryptorameccMetricsIterator returns an iterator
func NewMdhensintdrbgcryptorameccMetricsIterator() (*MdhensintdrbgcryptorameccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("MdhensintdrbgcryptorameccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &MdhensintdrbgcryptorameccMetricsIterator{iter: iter}, nil
}

type MdhensintaxierrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	WrspErr metrics.Counter

	RrspErr metrics.Counter

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
func (mtr *MdhensintaxierrMetrics) SetWrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrspErr"))
	return nil
}

// SetRrspErr sets cunter in shared memory
func (mtr *MdhensintaxierrMetrics) SetRrspErr(val metrics.Counter) error {
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
	if mtr == nil {
		return nil
	}

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

// Free frees the iterator memory
func (it *MdhensintaxierrMetricsIterator) Free() {
	it.iter.Free()
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

type TpcpicsintpicsMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	UncorrectableEcc metrics.Counter

	CorrectableEcc metrics.Counter

	TooManyRlSchError metrics.Counter

	Picc metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *TpcpicsintpicsMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *TpcpicsintpicsMetrics) Size() int {
	sz := 0

	sz += mtr.UncorrectableEcc.Size()

	sz += mtr.CorrectableEcc.Size()

	sz += mtr.TooManyRlSchError.Size()

	sz += mtr.Picc.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *TpcpicsintpicsMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.UncorrectableEcc = mtr.metrics.GetCounter(offset)
	offset += mtr.UncorrectableEcc.Size()

	mtr.CorrectableEcc = mtr.metrics.GetCounter(offset)
	offset += mtr.CorrectableEcc.Size()

	mtr.TooManyRlSchError = mtr.metrics.GetCounter(offset)
	offset += mtr.TooManyRlSchError.Size()

	mtr.Picc = mtr.metrics.GetCounter(offset)
	offset += mtr.Picc.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *TpcpicsintpicsMetrics) getOffset(fldName string) int {
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

	if fldName == "Picc" {
		return offset
	}
	offset += mtr.Picc.Size()

	return offset
}

// SetUncorrectableEcc sets cunter in shared memory
func (mtr *TpcpicsintpicsMetrics) SetUncorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UncorrectableEcc"))
	return nil
}

// SetCorrectableEcc sets cunter in shared memory
func (mtr *TpcpicsintpicsMetrics) SetCorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CorrectableEcc"))
	return nil
}

// SetTooManyRlSchError sets cunter in shared memory
func (mtr *TpcpicsintpicsMetrics) SetTooManyRlSchError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TooManyRlSchError"))
	return nil
}

// SetPicc sets cunter in shared memory
func (mtr *TpcpicsintpicsMetrics) SetPicc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Picc"))
	return nil
}

// TpcpicsintpicsMetricsIterator is the iterator object
type TpcpicsintpicsMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *TpcpicsintpicsMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *TpcpicsintpicsMetricsIterator) Next() *TpcpicsintpicsMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &TpcpicsintpicsMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *TpcpicsintpicsMetricsIterator) Find(key uint64) (*TpcpicsintpicsMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &TpcpicsintpicsMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *TpcpicsintpicsMetricsIterator) Create(key uint64) (*TpcpicsintpicsMetrics, error) {
	tmtr := &TpcpicsintpicsMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &TpcpicsintpicsMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *TpcpicsintpicsMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *TpcpicsintpicsMetricsIterator) Free() {
	it.iter.Free()
}

// NewTpcpicsintpicsMetricsIterator returns an iterator
func NewTpcpicsintpicsMetricsIterator() (*TpcpicsintpicsMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("TpcpicsintpicsMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &TpcpicsintpicsMetricsIterator{iter: iter}, nil
}

type TpcpicsintbadaddrMetrics struct {
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

func (mtr *TpcpicsintbadaddrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *TpcpicsintbadaddrMetrics) Size() int {
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
func (mtr *TpcpicsintbadaddrMetrics) Unmarshal() error {
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
func (mtr *TpcpicsintbadaddrMetrics) getOffset(fldName string) int {
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
func (mtr *TpcpicsintbadaddrMetrics) SetRdreq0BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq0BadAddr"))
	return nil
}

// SetWrreq0BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetWrreq0BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq0BadAddr"))
	return nil
}

// SetRdreq1BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetRdreq1BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq1BadAddr"))
	return nil
}

// SetWrreq1BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetWrreq1BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq1BadAddr"))
	return nil
}

// SetRdreq2BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetRdreq2BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq2BadAddr"))
	return nil
}

// SetWrreq2BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetWrreq2BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq2BadAddr"))
	return nil
}

// SetRdreq3BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetRdreq3BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq3BadAddr"))
	return nil
}

// SetWrreq3BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetWrreq3BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq3BadAddr"))
	return nil
}

// SetRdreq4BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetRdreq4BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq4BadAddr"))
	return nil
}

// SetWrreq4BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetWrreq4BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq4BadAddr"))
	return nil
}

// SetRdreq5BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetRdreq5BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq5BadAddr"))
	return nil
}

// SetWrreq5BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetWrreq5BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq5BadAddr"))
	return nil
}

// SetRdreq6BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetRdreq6BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq6BadAddr"))
	return nil
}

// SetWrreq6BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetWrreq6BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq6BadAddr"))
	return nil
}

// SetRdreq7BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetRdreq7BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq7BadAddr"))
	return nil
}

// SetWrreq7BadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetWrreq7BadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq7BadAddr"))
	return nil
}

// SetCpuBadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetCpuBadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CpuBadAddr"))
	return nil
}

// SetBgBadAddr sets cunter in shared memory
func (mtr *TpcpicsintbadaddrMetrics) SetBgBadAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BgBadAddr"))
	return nil
}

// TpcpicsintbadaddrMetricsIterator is the iterator object
type TpcpicsintbadaddrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *TpcpicsintbadaddrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *TpcpicsintbadaddrMetricsIterator) Next() *TpcpicsintbadaddrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &TpcpicsintbadaddrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *TpcpicsintbadaddrMetricsIterator) Find(key uint64) (*TpcpicsintbadaddrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &TpcpicsintbadaddrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *TpcpicsintbadaddrMetricsIterator) Create(key uint64) (*TpcpicsintbadaddrMetrics, error) {
	tmtr := &TpcpicsintbadaddrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &TpcpicsintbadaddrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *TpcpicsintbadaddrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *TpcpicsintbadaddrMetricsIterator) Free() {
	it.iter.Free()
}

// NewTpcpicsintbadaddrMetricsIterator returns an iterator
func NewTpcpicsintbadaddrMetricsIterator() (*TpcpicsintbadaddrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("TpcpicsintbadaddrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &TpcpicsintbadaddrMetricsIterator{iter: iter}, nil
}

type TpcpicsintbgMetrics struct {
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

func (mtr *TpcpicsintbgMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *TpcpicsintbgMetrics) Size() int {
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
func (mtr *TpcpicsintbgMetrics) Unmarshal() error {
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
func (mtr *TpcpicsintbgMetrics) getOffset(fldName string) int {
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
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg0"))
	return nil
}

// SetUnfinishedBg1 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg1"))
	return nil
}

// SetUnfinishedBg2 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg2"))
	return nil
}

// SetUnfinishedBg3 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg3"))
	return nil
}

// SetUnfinishedBg4 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg4"))
	return nil
}

// SetUnfinishedBg5 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg5"))
	return nil
}

// SetUnfinishedBg6 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg6"))
	return nil
}

// SetUnfinishedBg7 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg7"))
	return nil
}

// SetUnfinishedBg8 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg8"))
	return nil
}

// SetUnfinishedBg9 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg9"))
	return nil
}

// SetUnfinishedBg10 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg10"))
	return nil
}

// SetUnfinishedBg11 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg11"))
	return nil
}

// SetUnfinishedBg12 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg12(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg12"))
	return nil
}

// SetUnfinishedBg13 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg13(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg13"))
	return nil
}

// SetUnfinishedBg14 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg14(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg14"))
	return nil
}

// SetUnfinishedBg15 sets cunter in shared memory
func (mtr *TpcpicsintbgMetrics) SetUnfinishedBg15(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg15"))
	return nil
}

// TpcpicsintbgMetricsIterator is the iterator object
type TpcpicsintbgMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *TpcpicsintbgMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *TpcpicsintbgMetricsIterator) Next() *TpcpicsintbgMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &TpcpicsintbgMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *TpcpicsintbgMetricsIterator) Find(key uint64) (*TpcpicsintbgMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &TpcpicsintbgMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *TpcpicsintbgMetricsIterator) Create(key uint64) (*TpcpicsintbgMetrics, error) {
	tmtr := &TpcpicsintbgMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &TpcpicsintbgMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *TpcpicsintbgMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *TpcpicsintbgMetricsIterator) Free() {
	it.iter.Free()
}

// NewTpcpicsintbgMetricsIterator returns an iterator
func NewTpcpicsintbgMetricsIterator() (*TpcpicsintbgMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("TpcpicsintbgMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &TpcpicsintbgMetricsIterator{iter: iter}, nil
}

type TpcpicspiccintpiccMetrics struct {
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

func (mtr *TpcpicspiccintpiccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *TpcpicspiccintpiccMetrics) Size() int {
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
func (mtr *TpcpicspiccintpiccMetrics) Unmarshal() error {
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
func (mtr *TpcpicspiccintpiccMetrics) getOffset(fldName string) int {
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
func (mtr *TpcpicspiccintpiccMetrics) SetUncorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UncorrectableEcc"))
	return nil
}

// SetCorrectableEcc sets cunter in shared memory
func (mtr *TpcpicspiccintpiccMetrics) SetCorrectableEcc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CorrectableEcc"))
	return nil
}

// SetCacheStgAwlenErr sets cunter in shared memory
func (mtr *TpcpicspiccintpiccMetrics) SetCacheStgAwlenErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheStgAwlenErr"))
	return nil
}

// SetCacheHbmRrespSlverr sets cunter in shared memory
func (mtr *TpcpicspiccintpiccMetrics) SetCacheHbmRrespSlverr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmRrespSlverr"))
	return nil
}

// SetCacheHbmRrespDecerr sets cunter in shared memory
func (mtr *TpcpicspiccintpiccMetrics) SetCacheHbmRrespDecerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmRrespDecerr"))
	return nil
}

// SetCacheHbmBrespSlverr sets cunter in shared memory
func (mtr *TpcpicspiccintpiccMetrics) SetCacheHbmBrespSlverr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmBrespSlverr"))
	return nil
}

// SetCacheHbmBrespDecerr sets cunter in shared memory
func (mtr *TpcpicspiccintpiccMetrics) SetCacheHbmBrespDecerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheHbmBrespDecerr"))
	return nil
}

// TpcpicspiccintpiccMetricsIterator is the iterator object
type TpcpicspiccintpiccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *TpcpicspiccintpiccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *TpcpicspiccintpiccMetricsIterator) Next() *TpcpicspiccintpiccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &TpcpicspiccintpiccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *TpcpicspiccintpiccMetricsIterator) Find(key uint64) (*TpcpicspiccintpiccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &TpcpicspiccintpiccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *TpcpicspiccintpiccMetricsIterator) Create(key uint64) (*TpcpicspiccintpiccMetrics, error) {
	tmtr := &TpcpicspiccintpiccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &TpcpicspiccintpiccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *TpcpicspiccintpiccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *TpcpicspiccintpiccMetricsIterator) Free() {
	it.iter.Free()
}

// NewTpcpicspiccintpiccMetricsIterator returns an iterator
func NewTpcpicspiccintpiccMetricsIterator() (*TpcpicspiccintpiccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("TpcpicspiccintpiccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &TpcpicspiccintpiccMetricsIterator{iter: iter}, nil
}

type Dprdpr0intreg1Metrics struct {
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

	ErrCrcMaskOffsetGtPktSize metrics.Counter

	ErrPktEopEarly metrics.Counter

	ErrPtrFfOverflow metrics.Counter

	ErrCsumFfOverflow metrics.Counter

	ErrPktoutFfOverflow metrics.Counter

	ErrPtrFromCfgOverflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr0intreg1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr0intreg1Metrics) Size() int {
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
func (mtr *Dprdpr0intreg1Metrics) Unmarshal() error {
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
func (mtr *Dprdpr0intreg1Metrics) getOffset(fldName string) int {
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
func (mtr *Dprdpr0intreg1Metrics) SetErrPhvSopNoEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvSopNoEop"))
	return nil
}

// SetErrPhvEopNoSop sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrPhvEopNoSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvEopNoSop"))
	return nil
}

// SetErrOhiSopNoEop sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrOhiSopNoEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrOhiSopNoEop"))
	return nil
}

// SetErrOhiEopNoSop sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrOhiEopNoSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrOhiEopNoSop"))
	return nil
}

// SetErrPktinSopNoEop sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrPktinSopNoEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPktinSopNoEop"))
	return nil
}

// SetErrPktinEopNoSop sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrPktinEopNoSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPktinEopNoSop"))
	return nil
}

// SetErrCsumOffsetGtPktSize_4 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumOffsetGtPktSize_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_4"))
	return nil
}

// SetErrCsumOffsetGtPktSize_3 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumOffsetGtPktSize_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_3"))
	return nil
}

// SetErrCsumOffsetGtPktSize_2 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumOffsetGtPktSize_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_2"))
	return nil
}

// SetErrCsumOffsetGtPktSize_1 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumOffsetGtPktSize_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_1"))
	return nil
}

// SetErrCsumOffsetGtPktSize_0 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumOffsetGtPktSize_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_0"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_4 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_4"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_3 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_3"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_2 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_2"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_1 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_1"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_0 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_0"))
	return nil
}

// SetErrCsumLocGtPktSize_4 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumLocGtPktSize_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_4"))
	return nil
}

// SetErrCsumLocGtPktSize_3 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumLocGtPktSize_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_3"))
	return nil
}

// SetErrCsumLocGtPktSize_2 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumLocGtPktSize_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_2"))
	return nil
}

// SetErrCsumLocGtPktSize_1 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumLocGtPktSize_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_1"))
	return nil
}

// SetErrCsumLocGtPktSize_0 sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumLocGtPktSize_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_0"))
	return nil
}

// SetErrCrcOffsetGtPktSize sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCrcOffsetGtPktSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcOffsetGtPktSize"))
	return nil
}

// SetErrCrcLocGtPktSize sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCrcLocGtPktSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcLocGtPktSize"))
	return nil
}

// SetErrCrcMaskOffsetGtPktSize sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCrcMaskOffsetGtPktSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcMaskOffsetGtPktSize"))
	return nil
}

// SetErrPktEopEarly sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrPktEopEarly(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPktEopEarly"))
	return nil
}

// SetErrPtrFfOverflow sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrPtrFfOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPtrFfOverflow"))
	return nil
}

// SetErrCsumFfOverflow sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrCsumFfOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumFfOverflow"))
	return nil
}

// SetErrPktoutFfOverflow sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrPktoutFfOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPktoutFfOverflow"))
	return nil
}

// SetErrPtrFromCfgOverflow sets cunter in shared memory
func (mtr *Dprdpr0intreg1Metrics) SetErrPtrFromCfgOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPtrFromCfgOverflow"))
	return nil
}

// Dprdpr0intreg1MetricsIterator is the iterator object
type Dprdpr0intreg1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr0intreg1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr0intreg1MetricsIterator) Next() *Dprdpr0intreg1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr0intreg1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr0intreg1MetricsIterator) Find(key uint64) (*Dprdpr0intreg1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr0intreg1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr0intreg1MetricsIterator) Create(key uint64) (*Dprdpr0intreg1Metrics, error) {
	tmtr := &Dprdpr0intreg1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr0intreg1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr0intreg1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr0intreg1MetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr0intreg1MetricsIterator returns an iterator
func NewDprdpr0intreg1MetricsIterator() (*Dprdpr0intreg1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr0intreg1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr0intreg1MetricsIterator{iter: iter}, nil
}

type Dprdpr0intreg2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	FieldC metrics.Counter

	FieldD metrics.Counter

	ErrCsumStartGtEnd_4 metrics.Counter

	ErrCsumStartGtEnd_3 metrics.Counter

	ErrCsumStartGtEnd_2 metrics.Counter

	ErrCsumStartGtEnd_1 metrics.Counter

	ErrCsumStartGtEnd_0 metrics.Counter

	ErrCrcStartGtEnd metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr0intreg2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr0intreg2Metrics) Size() int {
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
func (mtr *Dprdpr0intreg2Metrics) Unmarshal() error {
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
func (mtr *Dprdpr0intreg2Metrics) getOffset(fldName string) int {
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
func (mtr *Dprdpr0intreg2Metrics) SetFieldC(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FieldC"))
	return nil
}

// SetFieldD sets cunter in shared memory
func (mtr *Dprdpr0intreg2Metrics) SetFieldD(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FieldD"))
	return nil
}

// SetErrCsumStartGtEnd_4 sets cunter in shared memory
func (mtr *Dprdpr0intreg2Metrics) SetErrCsumStartGtEnd_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_4"))
	return nil
}

// SetErrCsumStartGtEnd_3 sets cunter in shared memory
func (mtr *Dprdpr0intreg2Metrics) SetErrCsumStartGtEnd_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_3"))
	return nil
}

// SetErrCsumStartGtEnd_2 sets cunter in shared memory
func (mtr *Dprdpr0intreg2Metrics) SetErrCsumStartGtEnd_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_2"))
	return nil
}

// SetErrCsumStartGtEnd_1 sets cunter in shared memory
func (mtr *Dprdpr0intreg2Metrics) SetErrCsumStartGtEnd_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_1"))
	return nil
}

// SetErrCsumStartGtEnd_0 sets cunter in shared memory
func (mtr *Dprdpr0intreg2Metrics) SetErrCsumStartGtEnd_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_0"))
	return nil
}

// SetErrCrcStartGtEnd sets cunter in shared memory
func (mtr *Dprdpr0intreg2Metrics) SetErrCrcStartGtEnd(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcStartGtEnd"))
	return nil
}

// Dprdpr0intreg2MetricsIterator is the iterator object
type Dprdpr0intreg2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr0intreg2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr0intreg2MetricsIterator) Next() *Dprdpr0intreg2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr0intreg2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr0intreg2MetricsIterator) Find(key uint64) (*Dprdpr0intreg2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr0intreg2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr0intreg2MetricsIterator) Create(key uint64) (*Dprdpr0intreg2Metrics, error) {
	tmtr := &Dprdpr0intreg2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr0intreg2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr0intreg2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr0intreg2MetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr0intreg2MetricsIterator returns an iterator
func NewDprdpr0intreg2MetricsIterator() (*Dprdpr0intreg2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr0intreg2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr0intreg2MetricsIterator{iter: iter}, nil
}

type Dprdpr0intsramseccMetrics struct {
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

func (mtr *Dprdpr0intsramseccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr0intsramseccMetrics) Size() int {
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
func (mtr *Dprdpr0intsramseccMetrics) Unmarshal() error {
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
func (mtr *Dprdpr0intsramseccMetrics) getOffset(fldName string) int {
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
func (mtr *Dprdpr0intsramseccMetrics) SetDprPktinFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPktinFifoUncorrectable"))
	return nil
}

// SetDprPktinFifoCorrectable sets cunter in shared memory
func (mtr *Dprdpr0intsramseccMetrics) SetDprPktinFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPktinFifoCorrectable"))
	return nil
}

// SetDprCsumFifoUncorrectable sets cunter in shared memory
func (mtr *Dprdpr0intsramseccMetrics) SetDprCsumFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumFifoUncorrectable"))
	return nil
}

// SetDprCsumFifoCorrectable sets cunter in shared memory
func (mtr *Dprdpr0intsramseccMetrics) SetDprCsumFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumFifoCorrectable"))
	return nil
}

// SetDprPhvFifoUncorrectable sets cunter in shared memory
func (mtr *Dprdpr0intsramseccMetrics) SetDprPhvFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPhvFifoUncorrectable"))
	return nil
}

// SetDprPhvFifoCorrectable sets cunter in shared memory
func (mtr *Dprdpr0intsramseccMetrics) SetDprPhvFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPhvFifoCorrectable"))
	return nil
}

// SetDprOhiFifoUncorrectable sets cunter in shared memory
func (mtr *Dprdpr0intsramseccMetrics) SetDprOhiFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprOhiFifoUncorrectable"))
	return nil
}

// SetDprOhiFifoCorrectable sets cunter in shared memory
func (mtr *Dprdpr0intsramseccMetrics) SetDprOhiFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprOhiFifoCorrectable"))
	return nil
}

// SetDprPtrFifoUncorrectable sets cunter in shared memory
func (mtr *Dprdpr0intsramseccMetrics) SetDprPtrFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPtrFifoUncorrectable"))
	return nil
}

// SetDprPtrFifoCorrectable sets cunter in shared memory
func (mtr *Dprdpr0intsramseccMetrics) SetDprPtrFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPtrFifoCorrectable"))
	return nil
}

// SetDprPktoutFifoUncorrectable sets cunter in shared memory
func (mtr *Dprdpr0intsramseccMetrics) SetDprPktoutFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPktoutFifoUncorrectable"))
	return nil
}

// SetDprPktoutFifoCorrectable sets cunter in shared memory
func (mtr *Dprdpr0intsramseccMetrics) SetDprPktoutFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPktoutFifoCorrectable"))
	return nil
}

// Dprdpr0intsramseccMetricsIterator is the iterator object
type Dprdpr0intsramseccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr0intsramseccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr0intsramseccMetricsIterator) Next() *Dprdpr0intsramseccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr0intsramseccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr0intsramseccMetricsIterator) Find(key uint64) (*Dprdpr0intsramseccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr0intsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr0intsramseccMetricsIterator) Create(key uint64) (*Dprdpr0intsramseccMetrics, error) {
	tmtr := &Dprdpr0intsramseccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr0intsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr0intsramseccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr0intsramseccMetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr0intsramseccMetricsIterator returns an iterator
func NewDprdpr0intsramseccMetricsIterator() (*Dprdpr0intsramseccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr0intsramseccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr0intsramseccMetricsIterator{iter: iter}, nil
}

type Dprdpr0intfifoMetrics struct {
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

func (mtr *Dprdpr0intfifoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr0intfifoMetrics) Size() int {
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
func (mtr *Dprdpr0intfifoMetrics) Unmarshal() error {
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
func (mtr *Dprdpr0intfifoMetrics) getOffset(fldName string) int {
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
func (mtr *Dprdpr0intfifoMetrics) SetPhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvFfOvflow"))
	return nil
}

// SetOhiFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intfifoMetrics) SetOhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiFfOvflow"))
	return nil
}

// SetPktinFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intfifoMetrics) SetPktinFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktinFfOvflow"))
	return nil
}

// SetPktoutFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intfifoMetrics) SetPktoutFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutFfUndflow"))
	return nil
}

// SetCsumFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intfifoMetrics) SetCsumFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumFfOvflow"))
	return nil
}

// SetPtrFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intfifoMetrics) SetPtrFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrFfOvflow"))
	return nil
}

// Dprdpr0intfifoMetricsIterator is the iterator object
type Dprdpr0intfifoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr0intfifoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr0intfifoMetricsIterator) Next() *Dprdpr0intfifoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr0intfifoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr0intfifoMetricsIterator) Find(key uint64) (*Dprdpr0intfifoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr0intfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr0intfifoMetricsIterator) Create(key uint64) (*Dprdpr0intfifoMetrics, error) {
	tmtr := &Dprdpr0intfifoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr0intfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr0intfifoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr0intfifoMetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr0intfifoMetricsIterator returns an iterator
func NewDprdpr0intfifoMetricsIterator() (*Dprdpr0intfifoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr0intfifoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr0intfifoMetricsIterator{iter: iter}, nil
}

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

	ErrCrcMaskOffsetGtPktSize metrics.Counter

	ErrPktEopEarly metrics.Counter

	ErrPtrFfOverflow metrics.Counter

	ErrCsumFfOverflow metrics.Counter

	ErrPktoutFfOverflow metrics.Counter

	ErrPtrFromCfgOverflow metrics.Counter

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

	sz += mtr.ErrCrcMaskOffsetGtPktSize.Size()

	sz += mtr.ErrPktEopEarly.Size()

	sz += mtr.ErrPtrFfOverflow.Size()

	sz += mtr.ErrCsumFfOverflow.Size()

	sz += mtr.ErrPktoutFfOverflow.Size()

	sz += mtr.ErrPtrFromCfgOverflow.Size()

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

// SetErrCrcMaskOffsetGtPktSize sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCrcMaskOffsetGtPktSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcMaskOffsetGtPktSize"))
	return nil
}

// SetErrPktEopEarly sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrPktEopEarly(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPktEopEarly"))
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

// SetErrPtrFromCfgOverflow sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrPtrFromCfgOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPtrFromCfgOverflow"))
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

	FieldC metrics.Counter

	FieldD metrics.Counter

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
func (mtr *Dprdpr1intreg2Metrics) Unmarshal() error {
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
func (mtr *Dprdpr1intreg2Metrics) getOffset(fldName string) int {
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
func (mtr *Dprdpr1intreg2Metrics) SetFieldC(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FieldC"))
	return nil
}

// SetFieldD sets cunter in shared memory
func (mtr *Dprdpr1intreg2Metrics) SetFieldD(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FieldD"))
	return nil
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

type Dppdpp0intreg2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	FieldC metrics.Counter

	FieldD metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppdpp0intreg2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppdpp0intreg2Metrics) Size() int {
	sz := 0

	sz += mtr.FieldC.Size()

	sz += mtr.FieldD.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dppdpp0intreg2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.FieldC = mtr.metrics.GetCounter(offset)
	offset += mtr.FieldC.Size()

	mtr.FieldD = mtr.metrics.GetCounter(offset)
	offset += mtr.FieldD.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dppdpp0intreg2Metrics) getOffset(fldName string) int {
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
func (mtr *Dppdpp0intreg2Metrics) SetFieldC(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FieldC"))
	return nil
}

// SetFieldD sets cunter in shared memory
func (mtr *Dppdpp0intreg2Metrics) SetFieldD(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FieldD"))
	return nil
}

// Dppdpp0intreg2MetricsIterator is the iterator object
type Dppdpp0intreg2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppdpp0intreg2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppdpp0intreg2MetricsIterator) Next() *Dppdpp0intreg2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dppdpp0intreg2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppdpp0intreg2MetricsIterator) Find(key uint64) (*Dppdpp0intreg2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppdpp0intreg2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppdpp0intreg2MetricsIterator) Create(key uint64) (*Dppdpp0intreg2Metrics, error) {
	tmtr := &Dppdpp0intreg2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppdpp0intreg2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppdpp0intreg2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dppdpp0intreg2MetricsIterator) Free() {
	it.iter.Free()
}

// NewDppdpp0intreg2MetricsIterator returns an iterator
func NewDppdpp0intreg2MetricsIterator() (*Dppdpp0intreg2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppdpp0intreg2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppdpp0intreg2MetricsIterator{iter: iter}, nil
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

type Dppdpp1intreg2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	FieldC metrics.Counter

	FieldD metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppdpp1intreg2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppdpp1intreg2Metrics) Size() int {
	sz := 0

	sz += mtr.FieldC.Size()

	sz += mtr.FieldD.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dppdpp1intreg2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.FieldC = mtr.metrics.GetCounter(offset)
	offset += mtr.FieldC.Size()

	mtr.FieldD = mtr.metrics.GetCounter(offset)
	offset += mtr.FieldD.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dppdpp1intreg2Metrics) getOffset(fldName string) int {
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
func (mtr *Dppdpp1intreg2Metrics) SetFieldC(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FieldC"))
	return nil
}

// SetFieldD sets cunter in shared memory
func (mtr *Dppdpp1intreg2Metrics) SetFieldD(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FieldD"))
	return nil
}

// Dppdpp1intreg2MetricsIterator is the iterator object
type Dppdpp1intreg2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppdpp1intreg2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppdpp1intreg2MetricsIterator) Next() *Dppdpp1intreg2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dppdpp1intreg2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppdpp1intreg2MetricsIterator) Find(key uint64) (*Dppdpp1intreg2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppdpp1intreg2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppdpp1intreg2MetricsIterator) Create(key uint64) (*Dppdpp1intreg2Metrics, error) {
	tmtr := &Dppdpp1intreg2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppdpp1intreg2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppdpp1intreg2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dppdpp1intreg2MetricsIterator) Free() {
	it.iter.Free()
}

// NewDppdpp1intreg2MetricsIterator returns an iterator
func NewDppdpp1intreg2MetricsIterator() (*Dppdpp1intreg2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppdpp1intreg2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppdpp1intreg2MetricsIterator{iter: iter}, nil
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

type SemasemaintgroupsintregMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SemaErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *SemasemaintgroupsintregMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *SemasemaintgroupsintregMetrics) Size() int {
	sz := 0

	sz += mtr.SemaErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *SemasemaintgroupsintregMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SemaErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SemaErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *SemasemaintgroupsintregMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SemaErr" {
		return offset
	}
	offset += mtr.SemaErr.Size()

	return offset
}

// SetSemaErr sets cunter in shared memory
func (mtr *SemasemaintgroupsintregMetrics) SetSemaErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SemaErr"))
	return nil
}

// SemasemaintgroupsintregMetricsIterator is the iterator object
type SemasemaintgroupsintregMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SemasemaintgroupsintregMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SemasemaintgroupsintregMetricsIterator) Next() *SemasemaintgroupsintregMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &SemasemaintgroupsintregMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SemasemaintgroupsintregMetricsIterator) Find(key uint64) (*SemasemaintgroupsintregMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &SemasemaintgroupsintregMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SemasemaintgroupsintregMetricsIterator) Create(key uint64) (*SemasemaintgroupsintregMetrics, error) {
	tmtr := &SemasemaintgroupsintregMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &SemasemaintgroupsintregMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SemasemaintgroupsintregMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *SemasemaintgroupsintregMetricsIterator) Free() {
	it.iter.Free()
}

// NewSemasemaintgroupsintregMetricsIterator returns an iterator
func NewSemasemaintgroupsintregMetricsIterator() (*SemasemaintgroupsintregMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SemasemaintgroupsintregMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SemasemaintgroupsintregMetricsIterator{iter: iter}, nil
}

type MpmpnsintcryptoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Mpp0 metrics.Counter

	Mpp1 metrics.Counter

	Mpp2 metrics.Counter

	Mpp3 metrics.Counter

	Mpp4 metrics.Counter

	Mpp5 metrics.Counter

	Mpp6 metrics.Counter

	Mpp7 metrics.Counter

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
func (mtr *MpmpnsintcryptoMetrics) SetMpp0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp0"))
	return nil
}

// SetMpp1 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp1"))
	return nil
}

// SetMpp2 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp2"))
	return nil
}

// SetMpp3 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp3"))
	return nil
}

// SetMpp4 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp4"))
	return nil
}

// SetMpp5 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp5"))
	return nil
}

// SetMpp6 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp6"))
	return nil
}

// SetMpp7 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp7(val metrics.Counter) error {
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
	if mtr == nil {
		return nil
	}

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

// Free frees the iterator memory
func (it *MpmpnsintcryptoMetricsIterator) Free() {
	it.iter.Free()
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

	IntWrStallHit0Interrupt metrics.Counter

	IntRdStallHit0Interrupt metrics.Counter

	IntWrStallHit1Interrupt metrics.Counter

	IntRdStallHit1Interrupt metrics.Counter

	IntWrStallHit2Interrupt metrics.Counter

	IntRdStallHit2Interrupt metrics.Counter

	IntWrFull0Interrupt metrics.Counter

	IntRdFull0Interrupt metrics.Counter

	IntWrFull1Interrupt metrics.Counter

	IntRdFull1Interrupt metrics.Counter

	IntWrFull2Interrupt metrics.Counter

	IntRdFull2Interrupt metrics.Counter

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

	sz += mtr.IntWrStallHit0Interrupt.Size()

	sz += mtr.IntRdStallHit0Interrupt.Size()

	sz += mtr.IntWrStallHit1Interrupt.Size()

	sz += mtr.IntRdStallHit1Interrupt.Size()

	sz += mtr.IntWrStallHit2Interrupt.Size()

	sz += mtr.IntRdStallHit2Interrupt.Size()

	sz += mtr.IntWrFull0Interrupt.Size()

	sz += mtr.IntRdFull0Interrupt.Size()

	sz += mtr.IntWrFull1Interrupt.Size()

	sz += mtr.IntRdFull1Interrupt.Size()

	sz += mtr.IntWrFull2Interrupt.Size()

	sz += mtr.IntRdFull2Interrupt.Size()

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

	mtr.IntWrStallHit0Interrupt = mtr.metrics.GetCounter(offset)
	offset += mtr.IntWrStallHit0Interrupt.Size()

	mtr.IntRdStallHit0Interrupt = mtr.metrics.GetCounter(offset)
	offset += mtr.IntRdStallHit0Interrupt.Size()

	mtr.IntWrStallHit1Interrupt = mtr.metrics.GetCounter(offset)
	offset += mtr.IntWrStallHit1Interrupt.Size()

	mtr.IntRdStallHit1Interrupt = mtr.metrics.GetCounter(offset)
	offset += mtr.IntRdStallHit1Interrupt.Size()

	mtr.IntWrStallHit2Interrupt = mtr.metrics.GetCounter(offset)
	offset += mtr.IntWrStallHit2Interrupt.Size()

	mtr.IntRdStallHit2Interrupt = mtr.metrics.GetCounter(offset)
	offset += mtr.IntRdStallHit2Interrupt.Size()

	mtr.IntWrFull0Interrupt = mtr.metrics.GetCounter(offset)
	offset += mtr.IntWrFull0Interrupt.Size()

	mtr.IntRdFull0Interrupt = mtr.metrics.GetCounter(offset)
	offset += mtr.IntRdFull0Interrupt.Size()

	mtr.IntWrFull1Interrupt = mtr.metrics.GetCounter(offset)
	offset += mtr.IntWrFull1Interrupt.Size()

	mtr.IntRdFull1Interrupt = mtr.metrics.GetCounter(offset)
	offset += mtr.IntRdFull1Interrupt.Size()

	mtr.IntWrFull2Interrupt = mtr.metrics.GetCounter(offset)
	offset += mtr.IntWrFull2Interrupt.Size()

	mtr.IntRdFull2Interrupt = mtr.metrics.GetCounter(offset)
	offset += mtr.IntRdFull2Interrupt.Size()

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

	if fldName == "IntWrStallHit0Interrupt" {
		return offset
	}
	offset += mtr.IntWrStallHit0Interrupt.Size()

	if fldName == "IntRdStallHit0Interrupt" {
		return offset
	}
	offset += mtr.IntRdStallHit0Interrupt.Size()

	if fldName == "IntWrStallHit1Interrupt" {
		return offset
	}
	offset += mtr.IntWrStallHit1Interrupt.Size()

	if fldName == "IntRdStallHit1Interrupt" {
		return offset
	}
	offset += mtr.IntRdStallHit1Interrupt.Size()

	if fldName == "IntWrStallHit2Interrupt" {
		return offset
	}
	offset += mtr.IntWrStallHit2Interrupt.Size()

	if fldName == "IntRdStallHit2Interrupt" {
		return offset
	}
	offset += mtr.IntRdStallHit2Interrupt.Size()

	if fldName == "IntWrFull0Interrupt" {
		return offset
	}
	offset += mtr.IntWrFull0Interrupt.Size()

	if fldName == "IntRdFull0Interrupt" {
		return offset
	}
	offset += mtr.IntRdFull0Interrupt.Size()

	if fldName == "IntWrFull1Interrupt" {
		return offset
	}
	offset += mtr.IntWrFull1Interrupt.Size()

	if fldName == "IntRdFull1Interrupt" {
		return offset
	}
	offset += mtr.IntRdFull1Interrupt.Size()

	if fldName == "IntWrFull2Interrupt" {
		return offset
	}
	offset += mtr.IntWrFull2Interrupt.Size()

	if fldName == "IntRdFull2Interrupt" {
		return offset
	}
	offset += mtr.IntRdFull2Interrupt.Size()

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

// SetIntWrStallHit0Interrupt sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetIntWrStallHit0Interrupt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntWrStallHit0Interrupt"))
	return nil
}

// SetIntRdStallHit0Interrupt sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetIntRdStallHit0Interrupt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntRdStallHit0Interrupt"))
	return nil
}

// SetIntWrStallHit1Interrupt sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetIntWrStallHit1Interrupt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntWrStallHit1Interrupt"))
	return nil
}

// SetIntRdStallHit1Interrupt sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetIntRdStallHit1Interrupt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntRdStallHit1Interrupt"))
	return nil
}

// SetIntWrStallHit2Interrupt sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetIntWrStallHit2Interrupt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntWrStallHit2Interrupt"))
	return nil
}

// SetIntRdStallHit2Interrupt sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetIntRdStallHit2Interrupt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntRdStallHit2Interrupt"))
	return nil
}

// SetIntWrFull0Interrupt sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetIntWrFull0Interrupt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntWrFull0Interrupt"))
	return nil
}

// SetIntRdFull0Interrupt sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetIntRdFull0Interrupt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntRdFull0Interrupt"))
	return nil
}

// SetIntWrFull1Interrupt sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetIntWrFull1Interrupt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntWrFull1Interrupt"))
	return nil
}

// SetIntRdFull1Interrupt sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetIntRdFull1Interrupt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntRdFull1Interrupt"))
	return nil
}

// SetIntWrFull2Interrupt sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetIntWrFull2Interrupt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntWrFull2Interrupt"))
	return nil
}

// SetIntRdFull2Interrupt sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetIntRdFull2Interrupt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntRdFull2Interrupt"))
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

type MsmsintesecureMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MbRxint metrics.Counter

	MbTxint metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *MsmsintesecureMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *MsmsintesecureMetrics) Size() int {
	sz := 0

	sz += mtr.MbRxint.Size()

	sz += mtr.MbTxint.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *MsmsintesecureMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MbRxint = mtr.metrics.GetCounter(offset)
	offset += mtr.MbRxint.Size()

	mtr.MbTxint = mtr.metrics.GetCounter(offset)
	offset += mtr.MbTxint.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *MsmsintesecureMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MbRxint" {
		return offset
	}
	offset += mtr.MbRxint.Size()

	if fldName == "MbTxint" {
		return offset
	}
	offset += mtr.MbTxint.Size()

	return offset
}

// SetMbRxint sets cunter in shared memory
func (mtr *MsmsintesecureMetrics) SetMbRxint(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MbRxint"))
	return nil
}

// SetMbTxint sets cunter in shared memory
func (mtr *MsmsintesecureMetrics) SetMbTxint(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MbTxint"))
	return nil
}

// MsmsintesecureMetricsIterator is the iterator object
type MsmsintesecureMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *MsmsintesecureMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *MsmsintesecureMetricsIterator) Next() *MsmsintesecureMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &MsmsintesecureMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *MsmsintesecureMetricsIterator) Find(key uint64) (*MsmsintesecureMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &MsmsintesecureMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *MsmsintesecureMetricsIterator) Create(key uint64) (*MsmsintesecureMetrics, error) {
	tmtr := &MsmsintesecureMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &MsmsintesecureMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *MsmsintesecureMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *MsmsintesecureMetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintesecureMetricsIterator returns an iterator
func NewMsmsintesecureMetricsIterator() (*MsmsintesecureMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("MsmsintesecureMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &MsmsintesecureMetricsIterator{iter: iter}, nil
}

type Msmsintprp1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Read metrics.Counter

	Security metrics.Counter

	Decode metrics.Counter

	AckTimeout metrics.Counter

	StallTimeout metrics.Counter

	ByteRead metrics.Counter

	ByteWrite metrics.Counter

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

	sz += mtr.AckTimeout.Size()

	sz += mtr.StallTimeout.Size()

	sz += mtr.ByteRead.Size()

	sz += mtr.ByteWrite.Size()

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

	mtr.AckTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.AckTimeout.Size()

	mtr.StallTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.StallTimeout.Size()

	mtr.ByteRead = mtr.metrics.GetCounter(offset)
	offset += mtr.ByteRead.Size()

	mtr.ByteWrite = mtr.metrics.GetCounter(offset)
	offset += mtr.ByteWrite.Size()

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

	if fldName == "AckTimeout" {
		return offset
	}
	offset += mtr.AckTimeout.Size()

	if fldName == "StallTimeout" {
		return offset
	}
	offset += mtr.StallTimeout.Size()

	if fldName == "ByteRead" {
		return offset
	}
	offset += mtr.ByteRead.Size()

	if fldName == "ByteWrite" {
		return offset
	}
	offset += mtr.ByteWrite.Size()

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

// SetAckTimeout sets cunter in shared memory
func (mtr *Msmsintprp1Metrics) SetAckTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AckTimeout"))
	return nil
}

// SetStallTimeout sets cunter in shared memory
func (mtr *Msmsintprp1Metrics) SetStallTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StallTimeout"))
	return nil
}

// SetByteRead sets cunter in shared memory
func (mtr *Msmsintprp1Metrics) SetByteRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ByteRead"))
	return nil
}

// SetByteWrite sets cunter in shared memory
func (mtr *Msmsintprp1Metrics) SetByteWrite(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ByteWrite"))
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

	AckTimeout metrics.Counter

	StallTimeout metrics.Counter

	ByteRead metrics.Counter

	ByteWrite metrics.Counter

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

	sz += mtr.AckTimeout.Size()

	sz += mtr.StallTimeout.Size()

	sz += mtr.ByteRead.Size()

	sz += mtr.ByteWrite.Size()

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

	mtr.AckTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.AckTimeout.Size()

	mtr.StallTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.StallTimeout.Size()

	mtr.ByteRead = mtr.metrics.GetCounter(offset)
	offset += mtr.ByteRead.Size()

	mtr.ByteWrite = mtr.metrics.GetCounter(offset)
	offset += mtr.ByteWrite.Size()

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

	if fldName == "AckTimeout" {
		return offset
	}
	offset += mtr.AckTimeout.Size()

	if fldName == "StallTimeout" {
		return offset
	}
	offset += mtr.StallTimeout.Size()

	if fldName == "ByteRead" {
		return offset
	}
	offset += mtr.ByteRead.Size()

	if fldName == "ByteWrite" {
		return offset
	}
	offset += mtr.ByteWrite.Size()

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

// SetAckTimeout sets cunter in shared memory
func (mtr *Msmsintprp2Metrics) SetAckTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AckTimeout"))
	return nil
}

// SetStallTimeout sets cunter in shared memory
func (mtr *Msmsintprp2Metrics) SetStallTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StallTimeout"))
	return nil
}

// SetByteRead sets cunter in shared memory
func (mtr *Msmsintprp2Metrics) SetByteRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ByteRead"))
	return nil
}

// SetByteWrite sets cunter in shared memory
func (mtr *Msmsintprp2Metrics) SetByteWrite(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ByteWrite"))
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

	AckTimeout metrics.Counter

	StallTimeout metrics.Counter

	ByteRead metrics.Counter

	ByteWrite metrics.Counter

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

	sz += mtr.AckTimeout.Size()

	sz += mtr.StallTimeout.Size()

	sz += mtr.ByteRead.Size()

	sz += mtr.ByteWrite.Size()

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

	mtr.AckTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.AckTimeout.Size()

	mtr.StallTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.StallTimeout.Size()

	mtr.ByteRead = mtr.metrics.GetCounter(offset)
	offset += mtr.ByteRead.Size()

	mtr.ByteWrite = mtr.metrics.GetCounter(offset)
	offset += mtr.ByteWrite.Size()

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

	if fldName == "AckTimeout" {
		return offset
	}
	offset += mtr.AckTimeout.Size()

	if fldName == "StallTimeout" {
		return offset
	}
	offset += mtr.StallTimeout.Size()

	if fldName == "ByteRead" {
		return offset
	}
	offset += mtr.ByteRead.Size()

	if fldName == "ByteWrite" {
		return offset
	}
	offset += mtr.ByteWrite.Size()

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

// SetAckTimeout sets cunter in shared memory
func (mtr *Msmsintprp3Metrics) SetAckTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AckTimeout"))
	return nil
}

// SetStallTimeout sets cunter in shared memory
func (mtr *Msmsintprp3Metrics) SetStallTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StallTimeout"))
	return nil
}

// SetByteRead sets cunter in shared memory
func (mtr *Msmsintprp3Metrics) SetByteRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ByteRead"))
	return nil
}

// SetByteWrite sets cunter in shared memory
func (mtr *Msmsintprp3Metrics) SetByteWrite(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ByteWrite"))
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

	AckTimeout metrics.Counter

	StallTimeout metrics.Counter

	ByteRead metrics.Counter

	ByteWrite metrics.Counter

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

	sz += mtr.AckTimeout.Size()

	sz += mtr.StallTimeout.Size()

	sz += mtr.ByteRead.Size()

	sz += mtr.ByteWrite.Size()

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

	mtr.AckTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.AckTimeout.Size()

	mtr.StallTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.StallTimeout.Size()

	mtr.ByteRead = mtr.metrics.GetCounter(offset)
	offset += mtr.ByteRead.Size()

	mtr.ByteWrite = mtr.metrics.GetCounter(offset)
	offset += mtr.ByteWrite.Size()

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

	if fldName == "AckTimeout" {
		return offset
	}
	offset += mtr.AckTimeout.Size()

	if fldName == "StallTimeout" {
		return offset
	}
	offset += mtr.StallTimeout.Size()

	if fldName == "ByteRead" {
		return offset
	}
	offset += mtr.ByteRead.Size()

	if fldName == "ByteWrite" {
		return offset
	}
	offset += mtr.ByteWrite.Size()

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

// SetAckTimeout sets cunter in shared memory
func (mtr *Msmsintprp4Metrics) SetAckTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AckTimeout"))
	return nil
}

// SetStallTimeout sets cunter in shared memory
func (mtr *Msmsintprp4Metrics) SetStallTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StallTimeout"))
	return nil
}

// SetByteRead sets cunter in shared memory
func (mtr *Msmsintprp4Metrics) SetByteRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ByteRead"))
	return nil
}

// SetByteWrite sets cunter in shared memory
func (mtr *Msmsintprp4Metrics) SetByteWrite(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ByteWrite"))
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

	AckTimeout metrics.Counter

	StallTimeout metrics.Counter

	ByteRead metrics.Counter

	ByteWrite metrics.Counter

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

	sz += mtr.AckTimeout.Size()

	sz += mtr.StallTimeout.Size()

	sz += mtr.ByteRead.Size()

	sz += mtr.ByteWrite.Size()

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

	mtr.AckTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.AckTimeout.Size()

	mtr.StallTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.StallTimeout.Size()

	mtr.ByteRead = mtr.metrics.GetCounter(offset)
	offset += mtr.ByteRead.Size()

	mtr.ByteWrite = mtr.metrics.GetCounter(offset)
	offset += mtr.ByteWrite.Size()

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

	if fldName == "AckTimeout" {
		return offset
	}
	offset += mtr.AckTimeout.Size()

	if fldName == "StallTimeout" {
		return offset
	}
	offset += mtr.StallTimeout.Size()

	if fldName == "ByteRead" {
		return offset
	}
	offset += mtr.ByteRead.Size()

	if fldName == "ByteWrite" {
		return offset
	}
	offset += mtr.ByteWrite.Size()

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

// SetAckTimeout sets cunter in shared memory
func (mtr *Msmsintprp5Metrics) SetAckTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AckTimeout"))
	return nil
}

// SetStallTimeout sets cunter in shared memory
func (mtr *Msmsintprp5Metrics) SetStallTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StallTimeout"))
	return nil
}

// SetByteRead sets cunter in shared memory
func (mtr *Msmsintprp5Metrics) SetByteRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ByteRead"))
	return nil
}

// SetByteWrite sets cunter in shared memory
func (mtr *Msmsintprp5Metrics) SetByteWrite(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ByteWrite"))
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

type Msmsintgic0Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic0Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic0Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic0Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic0Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic0Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic0MetricsIterator is the iterator object
type Msmsintgic0MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic0MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic0MetricsIterator) Next() *Msmsintgic0Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic0Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic0MetricsIterator) Find(key uint64) (*Msmsintgic0Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic0MetricsIterator) Create(key uint64) (*Msmsintgic0Metrics, error) {
	tmtr := &Msmsintgic0Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic0MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic0MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic0MetricsIterator returns an iterator
func NewMsmsintgic0MetricsIterator() (*Msmsintgic0MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic0Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic0MetricsIterator{iter: iter}, nil
}

type Msmsintgic1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic1Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic1Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic1MetricsIterator is the iterator object
type Msmsintgic1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic1MetricsIterator) Next() *Msmsintgic1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic1MetricsIterator) Find(key uint64) (*Msmsintgic1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic1MetricsIterator) Create(key uint64) (*Msmsintgic1Metrics, error) {
	tmtr := &Msmsintgic1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic1MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic1MetricsIterator returns an iterator
func NewMsmsintgic1MetricsIterator() (*Msmsintgic1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic1MetricsIterator{iter: iter}, nil
}

type Msmsintgic2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic2Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic2Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic2Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic2MetricsIterator is the iterator object
type Msmsintgic2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic2MetricsIterator) Next() *Msmsintgic2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic2MetricsIterator) Find(key uint64) (*Msmsintgic2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic2MetricsIterator) Create(key uint64) (*Msmsintgic2Metrics, error) {
	tmtr := &Msmsintgic2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic2MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic2MetricsIterator returns an iterator
func NewMsmsintgic2MetricsIterator() (*Msmsintgic2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic2MetricsIterator{iter: iter}, nil
}

type Msmsintgic3Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic3Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic3Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic3Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic3Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic3Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic3MetricsIterator is the iterator object
type Msmsintgic3MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic3MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic3MetricsIterator) Next() *Msmsintgic3Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic3Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic3MetricsIterator) Find(key uint64) (*Msmsintgic3Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic3Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic3MetricsIterator) Create(key uint64) (*Msmsintgic3Metrics, error) {
	tmtr := &Msmsintgic3Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic3Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic3MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic3MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic3MetricsIterator returns an iterator
func NewMsmsintgic3MetricsIterator() (*Msmsintgic3MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic3Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic3MetricsIterator{iter: iter}, nil
}

type Msmsintgic4Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic4Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic4Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic4Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic4Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic4Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic4MetricsIterator is the iterator object
type Msmsintgic4MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic4MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic4MetricsIterator) Next() *Msmsintgic4Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic4Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic4MetricsIterator) Find(key uint64) (*Msmsintgic4Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic4Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic4MetricsIterator) Create(key uint64) (*Msmsintgic4Metrics, error) {
	tmtr := &Msmsintgic4Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic4Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic4MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic4MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic4MetricsIterator returns an iterator
func NewMsmsintgic4MetricsIterator() (*Msmsintgic4MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic4Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic4MetricsIterator{iter: iter}, nil
}

type Msmsintgic5Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic5Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic5Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic5Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic5Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic5Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic5MetricsIterator is the iterator object
type Msmsintgic5MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic5MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic5MetricsIterator) Next() *Msmsintgic5Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic5Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic5MetricsIterator) Find(key uint64) (*Msmsintgic5Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic5Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic5MetricsIterator) Create(key uint64) (*Msmsintgic5Metrics, error) {
	tmtr := &Msmsintgic5Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic5Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic5MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic5MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic5MetricsIterator returns an iterator
func NewMsmsintgic5MetricsIterator() (*Msmsintgic5MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic5Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic5MetricsIterator{iter: iter}, nil
}

type Msmsintgic6Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic6Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic6Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic6Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic6Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic6Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic6MetricsIterator is the iterator object
type Msmsintgic6MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic6MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic6MetricsIterator) Next() *Msmsintgic6Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic6Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic6MetricsIterator) Find(key uint64) (*Msmsintgic6Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic6Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic6MetricsIterator) Create(key uint64) (*Msmsintgic6Metrics, error) {
	tmtr := &Msmsintgic6Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic6Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic6MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic6MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic6MetricsIterator returns an iterator
func NewMsmsintgic6MetricsIterator() (*Msmsintgic6MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic6Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic6MetricsIterator{iter: iter}, nil
}

type Msmsintgic7Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic7Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic7Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic7Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic7Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic7Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic7MetricsIterator is the iterator object
type Msmsintgic7MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic7MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic7MetricsIterator) Next() *Msmsintgic7Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic7Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic7MetricsIterator) Find(key uint64) (*Msmsintgic7Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic7Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic7MetricsIterator) Create(key uint64) (*Msmsintgic7Metrics, error) {
	tmtr := &Msmsintgic7Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic7Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic7MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic7MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic7MetricsIterator returns an iterator
func NewMsmsintgic7MetricsIterator() (*Msmsintgic7MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic7Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic7MetricsIterator{iter: iter}, nil
}

type Msmsintgic8Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic8Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic8Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic8Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic8Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic8Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic8MetricsIterator is the iterator object
type Msmsintgic8MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic8MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic8MetricsIterator) Next() *Msmsintgic8Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic8Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic8MetricsIterator) Find(key uint64) (*Msmsintgic8Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic8Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic8MetricsIterator) Create(key uint64) (*Msmsintgic8Metrics, error) {
	tmtr := &Msmsintgic8Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic8Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic8MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic8MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic8MetricsIterator returns an iterator
func NewMsmsintgic8MetricsIterator() (*Msmsintgic8MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic8Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic8MetricsIterator{iter: iter}, nil
}

type Msmsintgic9Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic9Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic9Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic9Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic9Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic9Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic9MetricsIterator is the iterator object
type Msmsintgic9MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic9MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic9MetricsIterator) Next() *Msmsintgic9Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic9Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic9MetricsIterator) Find(key uint64) (*Msmsintgic9Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic9Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic9MetricsIterator) Create(key uint64) (*Msmsintgic9Metrics, error) {
	tmtr := &Msmsintgic9Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic9Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic9MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic9MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic9MetricsIterator returns an iterator
func NewMsmsintgic9MetricsIterator() (*Msmsintgic9MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic9Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic9MetricsIterator{iter: iter}, nil
}

type Msmsintgic10Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic10Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic10Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic10Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic10Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic10Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic10MetricsIterator is the iterator object
type Msmsintgic10MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic10MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic10MetricsIterator) Next() *Msmsintgic10Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic10Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic10MetricsIterator) Find(key uint64) (*Msmsintgic10Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic10Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic10MetricsIterator) Create(key uint64) (*Msmsintgic10Metrics, error) {
	tmtr := &Msmsintgic10Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic10Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic10MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic10MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic10MetricsIterator returns an iterator
func NewMsmsintgic10MetricsIterator() (*Msmsintgic10MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic10Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic10MetricsIterator{iter: iter}, nil
}

type Msmsintgic11Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic11Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic11Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic11Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic11Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic11Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic11MetricsIterator is the iterator object
type Msmsintgic11MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic11MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic11MetricsIterator) Next() *Msmsintgic11Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic11Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic11MetricsIterator) Find(key uint64) (*Msmsintgic11Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic11Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic11MetricsIterator) Create(key uint64) (*Msmsintgic11Metrics, error) {
	tmtr := &Msmsintgic11Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic11Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic11MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic11MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic11MetricsIterator returns an iterator
func NewMsmsintgic11MetricsIterator() (*Msmsintgic11MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic11Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic11MetricsIterator{iter: iter}, nil
}

type Msmsintgic12Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic12Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic12Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic12Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic12Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic12Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic12MetricsIterator is the iterator object
type Msmsintgic12MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic12MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic12MetricsIterator) Next() *Msmsintgic12Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic12Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic12MetricsIterator) Find(key uint64) (*Msmsintgic12Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic12Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic12MetricsIterator) Create(key uint64) (*Msmsintgic12Metrics, error) {
	tmtr := &Msmsintgic12Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic12Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic12MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic12MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic12MetricsIterator returns an iterator
func NewMsmsintgic12MetricsIterator() (*Msmsintgic12MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic12Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic12MetricsIterator{iter: iter}, nil
}

type Msmsintgic13Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic13Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic13Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic13Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic13Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic13Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic13MetricsIterator is the iterator object
type Msmsintgic13MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic13MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic13MetricsIterator) Next() *Msmsintgic13Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic13Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic13MetricsIterator) Find(key uint64) (*Msmsintgic13Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic13Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic13MetricsIterator) Create(key uint64) (*Msmsintgic13Metrics, error) {
	tmtr := &Msmsintgic13Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic13Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic13MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic13MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic13MetricsIterator returns an iterator
func NewMsmsintgic13MetricsIterator() (*Msmsintgic13MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic13Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic13MetricsIterator{iter: iter}, nil
}

type Msmsintgic14Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic14Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic14Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic14Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic14Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic14Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic14MetricsIterator is the iterator object
type Msmsintgic14MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic14MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic14MetricsIterator) Next() *Msmsintgic14Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic14Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic14MetricsIterator) Find(key uint64) (*Msmsintgic14Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic14Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic14MetricsIterator) Create(key uint64) (*Msmsintgic14Metrics, error) {
	tmtr := &Msmsintgic14Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic14Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic14MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic14MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic14MetricsIterator returns an iterator
func NewMsmsintgic14MetricsIterator() (*Msmsintgic14MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic14Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic14MetricsIterator{iter: iter}, nil
}

type Msmsintgic15Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic15Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic15Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic15Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic15Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic15Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic15MetricsIterator is the iterator object
type Msmsintgic15MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic15MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic15MetricsIterator) Next() *Msmsintgic15Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic15Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic15MetricsIterator) Find(key uint64) (*Msmsintgic15Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic15Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic15MetricsIterator) Create(key uint64) (*Msmsintgic15Metrics, error) {
	tmtr := &Msmsintgic15Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic15Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic15MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic15MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic15MetricsIterator returns an iterator
func NewMsmsintgic15MetricsIterator() (*Msmsintgic15MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic15Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic15MetricsIterator{iter: iter}, nil
}

type Msmsintgic16Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic16Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic16Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic16Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic16Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic16Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic16MetricsIterator is the iterator object
type Msmsintgic16MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic16MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic16MetricsIterator) Next() *Msmsintgic16Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic16Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic16MetricsIterator) Find(key uint64) (*Msmsintgic16Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic16Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic16MetricsIterator) Create(key uint64) (*Msmsintgic16Metrics, error) {
	tmtr := &Msmsintgic16Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic16Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic16MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic16MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic16MetricsIterator returns an iterator
func NewMsmsintgic16MetricsIterator() (*Msmsintgic16MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic16Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic16MetricsIterator{iter: iter}, nil
}

type Msmsintgic17Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic17Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic17Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic17Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic17Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic17Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic17MetricsIterator is the iterator object
type Msmsintgic17MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic17MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic17MetricsIterator) Next() *Msmsintgic17Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic17Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic17MetricsIterator) Find(key uint64) (*Msmsintgic17Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic17Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic17MetricsIterator) Create(key uint64) (*Msmsintgic17Metrics, error) {
	tmtr := &Msmsintgic17Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic17Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic17MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic17MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic17MetricsIterator returns an iterator
func NewMsmsintgic17MetricsIterator() (*Msmsintgic17MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic17Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic17MetricsIterator{iter: iter}, nil
}

type Msmsintgic18Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic18Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic18Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic18Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic18Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic18Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic18MetricsIterator is the iterator object
type Msmsintgic18MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic18MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic18MetricsIterator) Next() *Msmsintgic18Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic18Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic18MetricsIterator) Find(key uint64) (*Msmsintgic18Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic18Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic18MetricsIterator) Create(key uint64) (*Msmsintgic18Metrics, error) {
	tmtr := &Msmsintgic18Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic18Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic18MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic18MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic18MetricsIterator returns an iterator
func NewMsmsintgic18MetricsIterator() (*Msmsintgic18MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic18Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic18MetricsIterator{iter: iter}, nil
}

type Msmsintgic19Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic19Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic19Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic19Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic19Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic19Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic19MetricsIterator is the iterator object
type Msmsintgic19MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic19MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic19MetricsIterator) Next() *Msmsintgic19Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic19Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic19MetricsIterator) Find(key uint64) (*Msmsintgic19Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic19Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic19MetricsIterator) Create(key uint64) (*Msmsintgic19Metrics, error) {
	tmtr := &Msmsintgic19Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic19Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic19MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic19MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic19MetricsIterator returns an iterator
func NewMsmsintgic19MetricsIterator() (*Msmsintgic19MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic19Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic19MetricsIterator{iter: iter}, nil
}

type Msmsintgic20Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic20Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic20Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic20Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic20Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic20Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic20MetricsIterator is the iterator object
type Msmsintgic20MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic20MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic20MetricsIterator) Next() *Msmsintgic20Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic20Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic20MetricsIterator) Find(key uint64) (*Msmsintgic20Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic20Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic20MetricsIterator) Create(key uint64) (*Msmsintgic20Metrics, error) {
	tmtr := &Msmsintgic20Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic20Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic20MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic20MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic20MetricsIterator returns an iterator
func NewMsmsintgic20MetricsIterator() (*Msmsintgic20MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic20Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic20MetricsIterator{iter: iter}, nil
}

type Msmsintgic21Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic21Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic21Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic21Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic21Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic21Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic21MetricsIterator is the iterator object
type Msmsintgic21MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic21MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic21MetricsIterator) Next() *Msmsintgic21Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic21Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic21MetricsIterator) Find(key uint64) (*Msmsintgic21Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic21Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic21MetricsIterator) Create(key uint64) (*Msmsintgic21Metrics, error) {
	tmtr := &Msmsintgic21Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic21Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic21MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic21MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic21MetricsIterator returns an iterator
func NewMsmsintgic21MetricsIterator() (*Msmsintgic21MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic21Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic21MetricsIterator{iter: iter}, nil
}

type Msmsintgic22Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic22Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic22Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic22Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic22Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic22Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic22MetricsIterator is the iterator object
type Msmsintgic22MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic22MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic22MetricsIterator) Next() *Msmsintgic22Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic22Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic22MetricsIterator) Find(key uint64) (*Msmsintgic22Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic22Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic22MetricsIterator) Create(key uint64) (*Msmsintgic22Metrics, error) {
	tmtr := &Msmsintgic22Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic22Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic22MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic22MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic22MetricsIterator returns an iterator
func NewMsmsintgic22MetricsIterator() (*Msmsintgic22MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic22Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic22MetricsIterator{iter: iter}, nil
}

type Msmsintgic23Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MsarIntLvl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintgic23Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintgic23Metrics) Size() int {
	sz := 0

	sz += mtr.MsarIntLvl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintgic23Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MsarIntLvl = mtr.metrics.GetCounter(offset)
	offset += mtr.MsarIntLvl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintgic23Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MsarIntLvl" {
		return offset
	}
	offset += mtr.MsarIntLvl.Size()

	return offset
}

// SetMsarIntLvl sets cunter in shared memory
func (mtr *Msmsintgic23Metrics) SetMsarIntLvl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsarIntLvl"))
	return nil
}

// Msmsintgic23MetricsIterator is the iterator object
type Msmsintgic23MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintgic23MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintgic23MetricsIterator) Next() *Msmsintgic23Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintgic23Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintgic23MetricsIterator) Find(key uint64) (*Msmsintgic23Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintgic23Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintgic23MetricsIterator) Create(key uint64) (*Msmsintgic23Metrics, error) {
	tmtr := &Msmsintgic23Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintgic23Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintgic23MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintgic23MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintgic23MetricsIterator returns an iterator
func NewMsmsintgic23MetricsIterator() (*Msmsintgic23MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintgic23Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintgic23MetricsIterator{iter: iter}, nil
}

type MsemintgroupsintregMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Axi metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *MsemintgroupsintregMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *MsemintgroupsintregMetrics) Size() int {
	sz := 0

	sz += mtr.Axi.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *MsemintgroupsintregMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Axi = mtr.metrics.GetCounter(offset)
	offset += mtr.Axi.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *MsemintgroupsintregMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Axi" {
		return offset
	}
	offset += mtr.Axi.Size()

	return offset
}

// SetAxi sets cunter in shared memory
func (mtr *MsemintgroupsintregMetrics) SetAxi(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Axi"))
	return nil
}

// MsemintgroupsintregMetricsIterator is the iterator object
type MsemintgroupsintregMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *MsemintgroupsintregMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *MsemintgroupsintregMetricsIterator) Next() *MsemintgroupsintregMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &MsemintgroupsintregMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *MsemintgroupsintregMetricsIterator) Find(key uint64) (*MsemintgroupsintregMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &MsemintgroupsintregMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *MsemintgroupsintregMetricsIterator) Create(key uint64) (*MsemintgroupsintregMetrics, error) {
	tmtr := &MsemintgroupsintregMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &MsemintgroupsintregMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *MsemintgroupsintregMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *MsemintgroupsintregMetricsIterator) Free() {
	it.iter.Free()
}

// NewMsemintgroupsintregMetricsIterator returns an iterator
func NewMsemintgroupsintregMetricsIterator() (*MsemintgroupsintregMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("MsemintgroupsintregMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &MsemintgroupsintregMetricsIterator{iter: iter}, nil
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

	Umac_CFG3Intr metrics.Counter

	Umac_CFG3Pslverr metrics.Counter

	M0PbPbusDrdy metrics.Counter

	M1PbPbusDrdy metrics.Counter

	M2PbPbusDrdy metrics.Counter

	M3PbPbusDrdy metrics.Counter

	Rx0MissingSof metrics.Counter

	Rx0MissingEof metrics.Counter

	Rx0TimeoutErr metrics.Counter

	Rx0MinSizeErr metrics.Counter

	Rx1MissingSof metrics.Counter

	Rx1MissingEof metrics.Counter

	Rx1TimeoutErr metrics.Counter

	Rx1MinSizeErr metrics.Counter

	Rx2MissingSof metrics.Counter

	Rx2MissingEof metrics.Counter

	Rx2TimeoutErr metrics.Counter

	Rx2MinSizeErr metrics.Counter

	Rx3MissingSof metrics.Counter

	Rx3MissingEof metrics.Counter

	Rx3TimeoutErr metrics.Counter

	Rx3MinSizeErr metrics.Counter

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

	sz += mtr.Umac_CFG3Intr.Size()

	sz += mtr.Umac_CFG3Pslverr.Size()

	sz += mtr.M0PbPbusDrdy.Size()

	sz += mtr.M1PbPbusDrdy.Size()

	sz += mtr.M2PbPbusDrdy.Size()

	sz += mtr.M3PbPbusDrdy.Size()

	sz += mtr.Rx0MissingSof.Size()

	sz += mtr.Rx0MissingEof.Size()

	sz += mtr.Rx0TimeoutErr.Size()

	sz += mtr.Rx0MinSizeErr.Size()

	sz += mtr.Rx1MissingSof.Size()

	sz += mtr.Rx1MissingEof.Size()

	sz += mtr.Rx1TimeoutErr.Size()

	sz += mtr.Rx1MinSizeErr.Size()

	sz += mtr.Rx2MissingSof.Size()

	sz += mtr.Rx2MissingEof.Size()

	sz += mtr.Rx2TimeoutErr.Size()

	sz += mtr.Rx2MinSizeErr.Size()

	sz += mtr.Rx3MissingSof.Size()

	sz += mtr.Rx3MissingEof.Size()

	sz += mtr.Rx3TimeoutErr.Size()

	sz += mtr.Rx3MinSizeErr.Size()

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

	mtr.Umac_CFG3Intr = mtr.metrics.GetCounter(offset)
	offset += mtr.Umac_CFG3Intr.Size()

	mtr.Umac_CFG3Pslverr = mtr.metrics.GetCounter(offset)
	offset += mtr.Umac_CFG3Pslverr.Size()

	mtr.M0PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M0PbPbusDrdy.Size()

	mtr.M1PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M1PbPbusDrdy.Size()

	mtr.M2PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M2PbPbusDrdy.Size()

	mtr.M3PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M3PbPbusDrdy.Size()

	mtr.Rx0MissingSof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx0MissingSof.Size()

	mtr.Rx0MissingEof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx0MissingEof.Size()

	mtr.Rx0TimeoutErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx0TimeoutErr.Size()

	mtr.Rx0MinSizeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx0MinSizeErr.Size()

	mtr.Rx1MissingSof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx1MissingSof.Size()

	mtr.Rx1MissingEof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx1MissingEof.Size()

	mtr.Rx1TimeoutErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx1TimeoutErr.Size()

	mtr.Rx1MinSizeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx1MinSizeErr.Size()

	mtr.Rx2MissingSof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx2MissingSof.Size()

	mtr.Rx2MissingEof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx2MissingEof.Size()

	mtr.Rx2TimeoutErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx2TimeoutErr.Size()

	mtr.Rx2MinSizeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx2MinSizeErr.Size()

	mtr.Rx3MissingSof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx3MissingSof.Size()

	mtr.Rx3MissingEof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx3MissingEof.Size()

	mtr.Rx3TimeoutErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx3TimeoutErr.Size()

	mtr.Rx3MinSizeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx3MinSizeErr.Size()

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

	if fldName == "Umac_CFG3Intr" {
		return offset
	}
	offset += mtr.Umac_CFG3Intr.Size()

	if fldName == "Umac_CFG3Pslverr" {
		return offset
	}
	offset += mtr.Umac_CFG3Pslverr.Size()

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

	if fldName == "Rx0MissingSof" {
		return offset
	}
	offset += mtr.Rx0MissingSof.Size()

	if fldName == "Rx0MissingEof" {
		return offset
	}
	offset += mtr.Rx0MissingEof.Size()

	if fldName == "Rx0TimeoutErr" {
		return offset
	}
	offset += mtr.Rx0TimeoutErr.Size()

	if fldName == "Rx0MinSizeErr" {
		return offset
	}
	offset += mtr.Rx0MinSizeErr.Size()

	if fldName == "Rx1MissingSof" {
		return offset
	}
	offset += mtr.Rx1MissingSof.Size()

	if fldName == "Rx1MissingEof" {
		return offset
	}
	offset += mtr.Rx1MissingEof.Size()

	if fldName == "Rx1TimeoutErr" {
		return offset
	}
	offset += mtr.Rx1TimeoutErr.Size()

	if fldName == "Rx1MinSizeErr" {
		return offset
	}
	offset += mtr.Rx1MinSizeErr.Size()

	if fldName == "Rx2MissingSof" {
		return offset
	}
	offset += mtr.Rx2MissingSof.Size()

	if fldName == "Rx2MissingEof" {
		return offset
	}
	offset += mtr.Rx2MissingEof.Size()

	if fldName == "Rx2TimeoutErr" {
		return offset
	}
	offset += mtr.Rx2TimeoutErr.Size()

	if fldName == "Rx2MinSizeErr" {
		return offset
	}
	offset += mtr.Rx2MinSizeErr.Size()

	if fldName == "Rx3MissingSof" {
		return offset
	}
	offset += mtr.Rx3MissingSof.Size()

	if fldName == "Rx3MissingEof" {
		return offset
	}
	offset += mtr.Rx3MissingEof.Size()

	if fldName == "Rx3TimeoutErr" {
		return offset
	}
	offset += mtr.Rx3TimeoutErr.Size()

	if fldName == "Rx3MinSizeErr" {
		return offset
	}
	offset += mtr.Rx3MinSizeErr.Size()

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

// SetUmac_CFG3Intr sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetUmac_CFG3Intr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Umac_CFG3Intr"))
	return nil
}

// SetUmac_CFG3Pslverr sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetUmac_CFG3Pslverr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Umac_CFG3Pslverr"))
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

// SetRx0MissingSof sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx0MissingSof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx0MissingSof"))
	return nil
}

// SetRx0MissingEof sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx0MissingEof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx0MissingEof"))
	return nil
}

// SetRx0TimeoutErr sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx0TimeoutErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx0TimeoutErr"))
	return nil
}

// SetRx0MinSizeErr sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx0MinSizeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx0MinSizeErr"))
	return nil
}

// SetRx1MissingSof sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx1MissingSof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx1MissingSof"))
	return nil
}

// SetRx1MissingEof sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx1MissingEof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx1MissingEof"))
	return nil
}

// SetRx1TimeoutErr sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx1TimeoutErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx1TimeoutErr"))
	return nil
}

// SetRx1MinSizeErr sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx1MinSizeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx1MinSizeErr"))
	return nil
}

// SetRx2MissingSof sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx2MissingSof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx2MissingSof"))
	return nil
}

// SetRx2MissingEof sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx2MissingEof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx2MissingEof"))
	return nil
}

// SetRx2TimeoutErr sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx2TimeoutErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx2TimeoutErr"))
	return nil
}

// SetRx2MinSizeErr sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx2MinSizeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx2MinSizeErr"))
	return nil
}

// SetRx3MissingSof sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx3MissingSof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx3MissingSof"))
	return nil
}

// SetRx3MissingEof sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx3MissingEof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx3MissingEof"))
	return nil
}

// SetRx3TimeoutErr sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx3TimeoutErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx3TimeoutErr"))
	return nil
}

// SetRx3MinSizeErr sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetRx3MinSizeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx3MinSizeErr"))
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

	Umac_CFG3Intr metrics.Counter

	Umac_CFG3Pslverr metrics.Counter

	M0PbPbusDrdy metrics.Counter

	M1PbPbusDrdy metrics.Counter

	M2PbPbusDrdy metrics.Counter

	M3PbPbusDrdy metrics.Counter

	Rx0MissingSof metrics.Counter

	Rx0MissingEof metrics.Counter

	Rx0TimeoutErr metrics.Counter

	Rx0MinSizeErr metrics.Counter

	Rx1MissingSof metrics.Counter

	Rx1MissingEof metrics.Counter

	Rx1TimeoutErr metrics.Counter

	Rx1MinSizeErr metrics.Counter

	Rx2MissingSof metrics.Counter

	Rx2MissingEof metrics.Counter

	Rx2TimeoutErr metrics.Counter

	Rx2MinSizeErr metrics.Counter

	Rx3MissingSof metrics.Counter

	Rx3MissingEof metrics.Counter

	Rx3TimeoutErr metrics.Counter

	Rx3MinSizeErr metrics.Counter

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

	sz += mtr.Umac_CFG3Intr.Size()

	sz += mtr.Umac_CFG3Pslverr.Size()

	sz += mtr.M0PbPbusDrdy.Size()

	sz += mtr.M1PbPbusDrdy.Size()

	sz += mtr.M2PbPbusDrdy.Size()

	sz += mtr.M3PbPbusDrdy.Size()

	sz += mtr.Rx0MissingSof.Size()

	sz += mtr.Rx0MissingEof.Size()

	sz += mtr.Rx0TimeoutErr.Size()

	sz += mtr.Rx0MinSizeErr.Size()

	sz += mtr.Rx1MissingSof.Size()

	sz += mtr.Rx1MissingEof.Size()

	sz += mtr.Rx1TimeoutErr.Size()

	sz += mtr.Rx1MinSizeErr.Size()

	sz += mtr.Rx2MissingSof.Size()

	sz += mtr.Rx2MissingEof.Size()

	sz += mtr.Rx2TimeoutErr.Size()

	sz += mtr.Rx2MinSizeErr.Size()

	sz += mtr.Rx3MissingSof.Size()

	sz += mtr.Rx3MissingEof.Size()

	sz += mtr.Rx3TimeoutErr.Size()

	sz += mtr.Rx3MinSizeErr.Size()

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

	mtr.Umac_CFG3Intr = mtr.metrics.GetCounter(offset)
	offset += mtr.Umac_CFG3Intr.Size()

	mtr.Umac_CFG3Pslverr = mtr.metrics.GetCounter(offset)
	offset += mtr.Umac_CFG3Pslverr.Size()

	mtr.M0PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M0PbPbusDrdy.Size()

	mtr.M1PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M1PbPbusDrdy.Size()

	mtr.M2PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M2PbPbusDrdy.Size()

	mtr.M3PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M3PbPbusDrdy.Size()

	mtr.Rx0MissingSof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx0MissingSof.Size()

	mtr.Rx0MissingEof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx0MissingEof.Size()

	mtr.Rx0TimeoutErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx0TimeoutErr.Size()

	mtr.Rx0MinSizeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx0MinSizeErr.Size()

	mtr.Rx1MissingSof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx1MissingSof.Size()

	mtr.Rx1MissingEof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx1MissingEof.Size()

	mtr.Rx1TimeoutErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx1TimeoutErr.Size()

	mtr.Rx1MinSizeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx1MinSizeErr.Size()

	mtr.Rx2MissingSof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx2MissingSof.Size()

	mtr.Rx2MissingEof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx2MissingEof.Size()

	mtr.Rx2TimeoutErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx2TimeoutErr.Size()

	mtr.Rx2MinSizeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx2MinSizeErr.Size()

	mtr.Rx3MissingSof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx3MissingSof.Size()

	mtr.Rx3MissingEof = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx3MissingEof.Size()

	mtr.Rx3TimeoutErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx3TimeoutErr.Size()

	mtr.Rx3MinSizeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rx3MinSizeErr.Size()

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

	if fldName == "Umac_CFG3Intr" {
		return offset
	}
	offset += mtr.Umac_CFG3Intr.Size()

	if fldName == "Umac_CFG3Pslverr" {
		return offset
	}
	offset += mtr.Umac_CFG3Pslverr.Size()

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

	if fldName == "Rx0MissingSof" {
		return offset
	}
	offset += mtr.Rx0MissingSof.Size()

	if fldName == "Rx0MissingEof" {
		return offset
	}
	offset += mtr.Rx0MissingEof.Size()

	if fldName == "Rx0TimeoutErr" {
		return offset
	}
	offset += mtr.Rx0TimeoutErr.Size()

	if fldName == "Rx0MinSizeErr" {
		return offset
	}
	offset += mtr.Rx0MinSizeErr.Size()

	if fldName == "Rx1MissingSof" {
		return offset
	}
	offset += mtr.Rx1MissingSof.Size()

	if fldName == "Rx1MissingEof" {
		return offset
	}
	offset += mtr.Rx1MissingEof.Size()

	if fldName == "Rx1TimeoutErr" {
		return offset
	}
	offset += mtr.Rx1TimeoutErr.Size()

	if fldName == "Rx1MinSizeErr" {
		return offset
	}
	offset += mtr.Rx1MinSizeErr.Size()

	if fldName == "Rx2MissingSof" {
		return offset
	}
	offset += mtr.Rx2MissingSof.Size()

	if fldName == "Rx2MissingEof" {
		return offset
	}
	offset += mtr.Rx2MissingEof.Size()

	if fldName == "Rx2TimeoutErr" {
		return offset
	}
	offset += mtr.Rx2TimeoutErr.Size()

	if fldName == "Rx2MinSizeErr" {
		return offset
	}
	offset += mtr.Rx2MinSizeErr.Size()

	if fldName == "Rx3MissingSof" {
		return offset
	}
	offset += mtr.Rx3MissingSof.Size()

	if fldName == "Rx3MissingEof" {
		return offset
	}
	offset += mtr.Rx3MissingEof.Size()

	if fldName == "Rx3TimeoutErr" {
		return offset
	}
	offset += mtr.Rx3TimeoutErr.Size()

	if fldName == "Rx3MinSizeErr" {
		return offset
	}
	offset += mtr.Rx3MinSizeErr.Size()

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

// SetUmac_CFG3Intr sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetUmac_CFG3Intr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Umac_CFG3Intr"))
	return nil
}

// SetUmac_CFG3Pslverr sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetUmac_CFG3Pslverr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Umac_CFG3Pslverr"))
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

// SetRx0MissingSof sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx0MissingSof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx0MissingSof"))
	return nil
}

// SetRx0MissingEof sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx0MissingEof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx0MissingEof"))
	return nil
}

// SetRx0TimeoutErr sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx0TimeoutErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx0TimeoutErr"))
	return nil
}

// SetRx0MinSizeErr sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx0MinSizeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx0MinSizeErr"))
	return nil
}

// SetRx1MissingSof sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx1MissingSof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx1MissingSof"))
	return nil
}

// SetRx1MissingEof sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx1MissingEof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx1MissingEof"))
	return nil
}

// SetRx1TimeoutErr sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx1TimeoutErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx1TimeoutErr"))
	return nil
}

// SetRx1MinSizeErr sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx1MinSizeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx1MinSizeErr"))
	return nil
}

// SetRx2MissingSof sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx2MissingSof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx2MissingSof"))
	return nil
}

// SetRx2MissingEof sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx2MissingEof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx2MissingEof"))
	return nil
}

// SetRx2TimeoutErr sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx2TimeoutErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx2TimeoutErr"))
	return nil
}

// SetRx2MinSizeErr sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx2MinSizeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx2MinSizeErr"))
	return nil
}

// SetRx3MissingSof sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx3MissingSof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx3MissingSof"))
	return nil
}

// SetRx3MissingEof sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx3MissingEof(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx3MissingEof"))
	return nil
}

// SetRx3TimeoutErr sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx3TimeoutErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx3TimeoutErr"))
	return nil
}

// SetRx3MinSizeErr sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetRx3MinSizeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rx3MinSizeErr"))
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
