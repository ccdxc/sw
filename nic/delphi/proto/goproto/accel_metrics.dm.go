// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
)

type AccelSeqQueueKey struct {
	LifId uint64 `protobuf:"fixed64,1,opt,name=LifId,json=LifId" json:"LifId,omitempty"`

	QId uint32 `protobuf:"varint,2,opt,name=QId,json=QId" json:"QId,omitempty"`
}

func (m *AccelSeqQueueKey) Reset()         { *m = AccelSeqQueueKey{} }
func (m *AccelSeqQueueKey) String() string { return proto.CompactTextString(m) }
func (*AccelSeqQueueKey) ProtoMessage()    {}

type AccelSeqQueueMetrics struct {
	ObjectMeta api.ObjectMeta

	key AccelSeqQueueKey

	InterruptsRaised gometrics.Counter

	NextDBsRung gometrics.Counter

	SeqDescsProcessed gometrics.Counter

	SeqDescsAborted gometrics.Counter

	StatusPdmaXfers gometrics.Counter

	HwDescXfers gometrics.Counter

	HwBatchErrors gometrics.Counter

	HwOpErrors gometrics.Counter

	AolPadReqs gometrics.Counter

	SglPadReqs gometrics.Counter

	SglPdmaXfers gometrics.Counter

	SglPdmaErrors gometrics.Counter

	SglPadOnlyXfers gometrics.Counter

	SglPadOnlyErrors gometrics.Counter

	AltDescsTaken gometrics.Counter

	AltBufsTaken gometrics.Counter

	LenUpdateReqs gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *AccelSeqQueueMetrics) GetKey() AccelSeqQueueKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *AccelSeqQueueMetrics) Size() int {
	sz := 0

	sz += mtr.InterruptsRaised.Size()

	sz += mtr.NextDBsRung.Size()

	sz += mtr.SeqDescsProcessed.Size()

	sz += mtr.SeqDescsAborted.Size()

	sz += mtr.StatusPdmaXfers.Size()

	sz += mtr.HwDescXfers.Size()

	sz += mtr.HwBatchErrors.Size()

	sz += mtr.HwOpErrors.Size()

	sz += mtr.AolPadReqs.Size()

	sz += mtr.SglPadReqs.Size()

	sz += mtr.SglPdmaXfers.Size()

	sz += mtr.SglPdmaErrors.Size()

	sz += mtr.SglPadOnlyXfers.Size()

	sz += mtr.SglPadOnlyErrors.Size()

	sz += mtr.AltDescsTaken.Size()

	sz += mtr.AltBufsTaken.Size()

	sz += mtr.LenUpdateReqs.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *AccelSeqQueueMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.InterruptsRaised = mtr.metrics.GetCounter(offset)
	offset += mtr.InterruptsRaised.Size()

	mtr.NextDBsRung = mtr.metrics.GetCounter(offset)
	offset += mtr.NextDBsRung.Size()

	mtr.SeqDescsProcessed = mtr.metrics.GetCounter(offset)
	offset += mtr.SeqDescsProcessed.Size()

	mtr.SeqDescsAborted = mtr.metrics.GetCounter(offset)
	offset += mtr.SeqDescsAborted.Size()

	mtr.StatusPdmaXfers = mtr.metrics.GetCounter(offset)
	offset += mtr.StatusPdmaXfers.Size()

	mtr.HwDescXfers = mtr.metrics.GetCounter(offset)
	offset += mtr.HwDescXfers.Size()

	mtr.HwBatchErrors = mtr.metrics.GetCounter(offset)
	offset += mtr.HwBatchErrors.Size()

	mtr.HwOpErrors = mtr.metrics.GetCounter(offset)
	offset += mtr.HwOpErrors.Size()

	mtr.AolPadReqs = mtr.metrics.GetCounter(offset)
	offset += mtr.AolPadReqs.Size()

	mtr.SglPadReqs = mtr.metrics.GetCounter(offset)
	offset += mtr.SglPadReqs.Size()

	mtr.SglPdmaXfers = mtr.metrics.GetCounter(offset)
	offset += mtr.SglPdmaXfers.Size()

	mtr.SglPdmaErrors = mtr.metrics.GetCounter(offset)
	offset += mtr.SglPdmaErrors.Size()

	mtr.SglPadOnlyXfers = mtr.metrics.GetCounter(offset)
	offset += mtr.SglPadOnlyXfers.Size()

	mtr.SglPadOnlyErrors = mtr.metrics.GetCounter(offset)
	offset += mtr.SglPadOnlyErrors.Size()

	mtr.AltDescsTaken = mtr.metrics.GetCounter(offset)
	offset += mtr.AltDescsTaken.Size()

	mtr.AltBufsTaken = mtr.metrics.GetCounter(offset)
	offset += mtr.AltBufsTaken.Size()

	mtr.LenUpdateReqs = mtr.metrics.GetCounter(offset)
	offset += mtr.LenUpdateReqs.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *AccelSeqQueueMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "InterruptsRaised" {
		return offset
	}
	offset += mtr.InterruptsRaised.Size()

	if fldName == "NextDBsRung" {
		return offset
	}
	offset += mtr.NextDBsRung.Size()

	if fldName == "SeqDescsProcessed" {
		return offset
	}
	offset += mtr.SeqDescsProcessed.Size()

	if fldName == "SeqDescsAborted" {
		return offset
	}
	offset += mtr.SeqDescsAborted.Size()

	if fldName == "StatusPdmaXfers" {
		return offset
	}
	offset += mtr.StatusPdmaXfers.Size()

	if fldName == "HwDescXfers" {
		return offset
	}
	offset += mtr.HwDescXfers.Size()

	if fldName == "HwBatchErrors" {
		return offset
	}
	offset += mtr.HwBatchErrors.Size()

	if fldName == "HwOpErrors" {
		return offset
	}
	offset += mtr.HwOpErrors.Size()

	if fldName == "AolPadReqs" {
		return offset
	}
	offset += mtr.AolPadReqs.Size()

	if fldName == "SglPadReqs" {
		return offset
	}
	offset += mtr.SglPadReqs.Size()

	if fldName == "SglPdmaXfers" {
		return offset
	}
	offset += mtr.SglPdmaXfers.Size()

	if fldName == "SglPdmaErrors" {
		return offset
	}
	offset += mtr.SglPdmaErrors.Size()

	if fldName == "SglPadOnlyXfers" {
		return offset
	}
	offset += mtr.SglPadOnlyXfers.Size()

	if fldName == "SglPadOnlyErrors" {
		return offset
	}
	offset += mtr.SglPadOnlyErrors.Size()

	if fldName == "AltDescsTaken" {
		return offset
	}
	offset += mtr.AltDescsTaken.Size()

	if fldName == "AltBufsTaken" {
		return offset
	}
	offset += mtr.AltBufsTaken.Size()

	if fldName == "LenUpdateReqs" {
		return offset
	}
	offset += mtr.LenUpdateReqs.Size()

	return offset
}

// SetInterruptsRaised sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetInterruptsRaised(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("InterruptsRaised"))
	return nil
}

// SetNextDBsRung sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetNextDBsRung(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NextDBsRung"))
	return nil
}

// SetSeqDescsProcessed sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetSeqDescsProcessed(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SeqDescsProcessed"))
	return nil
}

// SetSeqDescsAborted sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetSeqDescsAborted(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SeqDescsAborted"))
	return nil
}

// SetStatusPdmaXfers sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetStatusPdmaXfers(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StatusPdmaXfers"))
	return nil
}

// SetHwDescXfers sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetHwDescXfers(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("HwDescXfers"))
	return nil
}

// SetHwBatchErrors sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetHwBatchErrors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("HwBatchErrors"))
	return nil
}

// SetHwOpErrors sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetHwOpErrors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("HwOpErrors"))
	return nil
}

// SetAolPadReqs sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetAolPadReqs(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AolPadReqs"))
	return nil
}

// SetSglPadReqs sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetSglPadReqs(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SglPadReqs"))
	return nil
}

// SetSglPdmaXfers sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetSglPdmaXfers(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SglPdmaXfers"))
	return nil
}

// SetSglPdmaErrors sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetSglPdmaErrors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SglPdmaErrors"))
	return nil
}

// SetSglPadOnlyXfers sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetSglPadOnlyXfers(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SglPadOnlyXfers"))
	return nil
}

// SetSglPadOnlyErrors sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetSglPadOnlyErrors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SglPadOnlyErrors"))
	return nil
}

// SetAltDescsTaken sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetAltDescsTaken(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AltDescsTaken"))
	return nil
}

// SetAltBufsTaken sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetAltBufsTaken(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AltBufsTaken"))
	return nil
}

// SetLenUpdateReqs sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) SetLenUpdateReqs(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LenUpdateReqs"))
	return nil
}

// AccelSeqQueueMetricsIterator is the iterator object
type AccelSeqQueueMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *AccelSeqQueueMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *AccelSeqQueueMetricsIterator) Next() *AccelSeqQueueMetrics {
	mtr := it.iter.Next()
	tmtr := &AccelSeqQueueMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *AccelSeqQueueMetricsIterator) Find(key AccelSeqQueueKey) (*AccelSeqQueueMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &AccelSeqQueueMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *AccelSeqQueueMetricsIterator) Create(key AccelSeqQueueKey) (*AccelSeqQueueMetrics, error) {
	tmtr := &AccelSeqQueueMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &AccelSeqQueueMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *AccelSeqQueueMetricsIterator) Delete(key AccelSeqQueueKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// NewAccelSeqQueueMetricsIterator returns an iterator
func NewAccelSeqQueueMetricsIterator() (*AccelSeqQueueMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("AccelSeqQueueMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &AccelSeqQueueMetricsIterator{iter: iter}, nil
}

type AccelHwRingKey struct {
	RId uint32 `protobuf:"varint,1,opt,name=RId,json=RId" json:"RId,omitempty"`

	SubRId uint32 `protobuf:"varint,2,opt,name=SubRId,json=SubRId" json:"SubRId,omitempty"`
}

func (m *AccelHwRingKey) Reset()         { *m = AccelHwRingKey{} }
func (m *AccelHwRingKey) String() string { return proto.CompactTextString(m) }
func (*AccelHwRingKey) ProtoMessage()    {}

type AccelHwRingMetrics struct {
	ObjectMeta api.ObjectMeta

	key AccelHwRingKey

	InputBytes gometrics.Counter

	OutputBytes gometrics.Counter

	SoftResets gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *AccelHwRingMetrics) GetKey() AccelHwRingKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *AccelHwRingMetrics) Size() int {
	sz := 0

	sz += mtr.InputBytes.Size()

	sz += mtr.OutputBytes.Size()

	sz += mtr.SoftResets.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *AccelHwRingMetrics) Unmarshal() error {
	var offset int

	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)

	mtr.InputBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.InputBytes.Size()

	mtr.OutputBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.OutputBytes.Size()

	mtr.SoftResets = mtr.metrics.GetCounter(offset)
	offset += mtr.SoftResets.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *AccelHwRingMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "InputBytes" {
		return offset
	}
	offset += mtr.InputBytes.Size()

	if fldName == "OutputBytes" {
		return offset
	}
	offset += mtr.OutputBytes.Size()

	if fldName == "SoftResets" {
		return offset
	}
	offset += mtr.SoftResets.Size()

	return offset
}

// SetInputBytes sets cunter in shared memory
func (mtr *AccelHwRingMetrics) SetInputBytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("InputBytes"))
	return nil
}

// SetOutputBytes sets cunter in shared memory
func (mtr *AccelHwRingMetrics) SetOutputBytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OutputBytes"))
	return nil
}

// SetSoftResets sets cunter in shared memory
func (mtr *AccelHwRingMetrics) SetSoftResets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SoftResets"))
	return nil
}

// AccelHwRingMetricsIterator is the iterator object
type AccelHwRingMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *AccelHwRingMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *AccelHwRingMetricsIterator) Next() *AccelHwRingMetrics {
	mtr := it.iter.Next()
	tmtr := &AccelHwRingMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *AccelHwRingMetricsIterator) Find(key AccelHwRingKey) (*AccelHwRingMetrics, error) {

	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)

	if err != nil {
		return nil, err
	}
	tmtr := &AccelHwRingMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *AccelHwRingMetricsIterator) Create(key AccelHwRingKey) (*AccelHwRingMetrics, error) {
	tmtr := &AccelHwRingMetrics{}

	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())

	tmtr = &AccelHwRingMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *AccelHwRingMetricsIterator) Delete(key AccelHwRingKey) error {

	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)

}

// NewAccelHwRingMetricsIterator returns an iterator
func NewAccelHwRingMetricsIterator() (*AccelHwRingMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("AccelHwRingMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &AccelHwRingMetricsIterator{iter: iter}, nil
}
