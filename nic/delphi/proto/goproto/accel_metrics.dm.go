// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"encoding/json"

	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
)

type AccelSeqQueueKey struct {
	lif_id uint64

	qid uint32
}

type AccelSeqQueueMetrics struct {
	key AccelSeqQueueKey

	interrupts_raised gometrics.Counter

	next_dbs_rung gometrics.Counter

	seq_descs_processed gometrics.Counter

	seq_descs_aborted gometrics.Counter

	status_pdma_xfers gometrics.Counter

	hw_desc_xfers gometrics.Counter

	hw_batch_errors gometrics.Counter

	hw_op_errors gometrics.Counter

	aol_pad_reqs gometrics.Counter

	sgl_pad_reqs gometrics.Counter

	sgl_pdma_xfers gometrics.Counter

	sgl_pdma_errors gometrics.Counter

	sgl_pad_only_xfers gometrics.Counter

	sgl_pad_only_errors gometrics.Counter

	alt_descs_taken gometrics.Counter

	alt_bufs_taken gometrics.Counter

	len_update_reqs gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *AccelSeqQueueMetrics) GetKey() AccelSeqQueueKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *AccelSeqQueueMetrics) Size() int {
	sz := 0

	sz += mtr.interrupts_raised.Size()

	sz += mtr.next_dbs_rung.Size()

	sz += mtr.seq_descs_processed.Size()

	sz += mtr.seq_descs_aborted.Size()

	sz += mtr.status_pdma_xfers.Size()

	sz += mtr.hw_desc_xfers.Size()

	sz += mtr.hw_batch_errors.Size()

	sz += mtr.hw_op_errors.Size()

	sz += mtr.aol_pad_reqs.Size()

	sz += mtr.sgl_pad_reqs.Size()

	sz += mtr.sgl_pdma_xfers.Size()

	sz += mtr.sgl_pdma_errors.Size()

	sz += mtr.sgl_pad_only_xfers.Size()

	sz += mtr.sgl_pad_only_errors.Size()

	sz += mtr.alt_descs_taken.Size()

	sz += mtr.alt_bufs_taken.Size()

	sz += mtr.len_update_reqs.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *AccelSeqQueueMetrics) Unmarshal() error {
	var offset int

	json.Unmarshal([]byte(mtr.metrics.GetKey()), &mtr.key)

	mtr.interrupts_raised = mtr.metrics.GetCounter(offset)
	offset += mtr.interrupts_raised.Size()

	mtr.next_dbs_rung = mtr.metrics.GetCounter(offset)
	offset += mtr.next_dbs_rung.Size()

	mtr.seq_descs_processed = mtr.metrics.GetCounter(offset)
	offset += mtr.seq_descs_processed.Size()

	mtr.seq_descs_aborted = mtr.metrics.GetCounter(offset)
	offset += mtr.seq_descs_aborted.Size()

	mtr.status_pdma_xfers = mtr.metrics.GetCounter(offset)
	offset += mtr.status_pdma_xfers.Size()

	mtr.hw_desc_xfers = mtr.metrics.GetCounter(offset)
	offset += mtr.hw_desc_xfers.Size()

	mtr.hw_batch_errors = mtr.metrics.GetCounter(offset)
	offset += mtr.hw_batch_errors.Size()

	mtr.hw_op_errors = mtr.metrics.GetCounter(offset)
	offset += mtr.hw_op_errors.Size()

	mtr.aol_pad_reqs = mtr.metrics.GetCounter(offset)
	offset += mtr.aol_pad_reqs.Size()

	mtr.sgl_pad_reqs = mtr.metrics.GetCounter(offset)
	offset += mtr.sgl_pad_reqs.Size()

	mtr.sgl_pdma_xfers = mtr.metrics.GetCounter(offset)
	offset += mtr.sgl_pdma_xfers.Size()

	mtr.sgl_pdma_errors = mtr.metrics.GetCounter(offset)
	offset += mtr.sgl_pdma_errors.Size()

	mtr.sgl_pad_only_xfers = mtr.metrics.GetCounter(offset)
	offset += mtr.sgl_pad_only_xfers.Size()

	mtr.sgl_pad_only_errors = mtr.metrics.GetCounter(offset)
	offset += mtr.sgl_pad_only_errors.Size()

	mtr.alt_descs_taken = mtr.metrics.GetCounter(offset)
	offset += mtr.alt_descs_taken.Size()

	mtr.alt_bufs_taken = mtr.metrics.GetCounter(offset)
	offset += mtr.alt_bufs_taken.Size()

	mtr.len_update_reqs = mtr.metrics.GetCounter(offset)
	offset += mtr.len_update_reqs.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *AccelSeqQueueMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "interrupts_raised" {
		return offset
	}
	offset += mtr.interrupts_raised.Size()

	if fldName == "next_dbs_rung" {
		return offset
	}
	offset += mtr.next_dbs_rung.Size()

	if fldName == "seq_descs_processed" {
		return offset
	}
	offset += mtr.seq_descs_processed.Size()

	if fldName == "seq_descs_aborted" {
		return offset
	}
	offset += mtr.seq_descs_aborted.Size()

	if fldName == "status_pdma_xfers" {
		return offset
	}
	offset += mtr.status_pdma_xfers.Size()

	if fldName == "hw_desc_xfers" {
		return offset
	}
	offset += mtr.hw_desc_xfers.Size()

	if fldName == "hw_batch_errors" {
		return offset
	}
	offset += mtr.hw_batch_errors.Size()

	if fldName == "hw_op_errors" {
		return offset
	}
	offset += mtr.hw_op_errors.Size()

	if fldName == "aol_pad_reqs" {
		return offset
	}
	offset += mtr.aol_pad_reqs.Size()

	if fldName == "sgl_pad_reqs" {
		return offset
	}
	offset += mtr.sgl_pad_reqs.Size()

	if fldName == "sgl_pdma_xfers" {
		return offset
	}
	offset += mtr.sgl_pdma_xfers.Size()

	if fldName == "sgl_pdma_errors" {
		return offset
	}
	offset += mtr.sgl_pdma_errors.Size()

	if fldName == "sgl_pad_only_xfers" {
		return offset
	}
	offset += mtr.sgl_pad_only_xfers.Size()

	if fldName == "sgl_pad_only_errors" {
		return offset
	}
	offset += mtr.sgl_pad_only_errors.Size()

	if fldName == "alt_descs_taken" {
		return offset
	}
	offset += mtr.alt_descs_taken.Size()

	if fldName == "alt_bufs_taken" {
		return offset
	}
	offset += mtr.alt_bufs_taken.Size()

	if fldName == "len_update_reqs" {
		return offset
	}
	offset += mtr.len_update_reqs.Size()

	return offset
}

// Setinterrupts_raised sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setinterrupts_raised(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("interrupts_raised"))
	return nil
}

// Setnext_dbs_rung sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setnext_dbs_rung(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("next_dbs_rung"))
	return nil
}

// Setseq_descs_processed sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setseq_descs_processed(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("seq_descs_processed"))
	return nil
}

// Setseq_descs_aborted sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setseq_descs_aborted(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("seq_descs_aborted"))
	return nil
}

// Setstatus_pdma_xfers sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setstatus_pdma_xfers(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("status_pdma_xfers"))
	return nil
}

// Sethw_desc_xfers sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Sethw_desc_xfers(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("hw_desc_xfers"))
	return nil
}

// Sethw_batch_errors sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Sethw_batch_errors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("hw_batch_errors"))
	return nil
}

// Sethw_op_errors sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Sethw_op_errors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("hw_op_errors"))
	return nil
}

// Setaol_pad_reqs sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setaol_pad_reqs(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("aol_pad_reqs"))
	return nil
}

// Setsgl_pad_reqs sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setsgl_pad_reqs(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("sgl_pad_reqs"))
	return nil
}

// Setsgl_pdma_xfers sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setsgl_pdma_xfers(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("sgl_pdma_xfers"))
	return nil
}

// Setsgl_pdma_errors sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setsgl_pdma_errors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("sgl_pdma_errors"))
	return nil
}

// Setsgl_pad_only_xfers sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setsgl_pad_only_xfers(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("sgl_pad_only_xfers"))
	return nil
}

// Setsgl_pad_only_errors sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setsgl_pad_only_errors(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("sgl_pad_only_errors"))
	return nil
}

// Setalt_descs_taken sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setalt_descs_taken(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("alt_descs_taken"))
	return nil
}

// Setalt_bufs_taken sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setalt_bufs_taken(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("alt_bufs_taken"))
	return nil
}

// Setlen_update_reqs sets cunter in shared memory
func (mtr *AccelSeqQueueMetrics) Setlen_update_reqs(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("len_update_reqs"))
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

	buf, _ := json.Marshal(key)
	mtr, err := it.iter.Find(string(buf))

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

	buf, _ := json.Marshal(key)
	mtr := it.iter.Create(string(buf), tmtr.Size())

	tmtr = &AccelSeqQueueMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *AccelSeqQueueMetricsIterator) Delete(key AccelSeqQueueKey) error {

	buf, _ := json.Marshal(key)
	return it.iter.Delete(string(buf))

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
	rid uint32

	sub_rid uint32
}

type AccelHwRingMetrics struct {
	key AccelHwRingKey

	input_bytes gometrics.Counter

	output_bytes gometrics.Counter

	soft_resets gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *AccelHwRingMetrics) GetKey() AccelHwRingKey {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *AccelHwRingMetrics) Size() int {
	sz := 0

	sz += mtr.input_bytes.Size()

	sz += mtr.output_bytes.Size()

	sz += mtr.soft_resets.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *AccelHwRingMetrics) Unmarshal() error {
	var offset int

	json.Unmarshal([]byte(mtr.metrics.GetKey()), &mtr.key)

	mtr.input_bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.input_bytes.Size()

	mtr.output_bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.output_bytes.Size()

	mtr.soft_resets = mtr.metrics.GetCounter(offset)
	offset += mtr.soft_resets.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *AccelHwRingMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "input_bytes" {
		return offset
	}
	offset += mtr.input_bytes.Size()

	if fldName == "output_bytes" {
		return offset
	}
	offset += mtr.output_bytes.Size()

	if fldName == "soft_resets" {
		return offset
	}
	offset += mtr.soft_resets.Size()

	return offset
}

// Setinput_bytes sets cunter in shared memory
func (mtr *AccelHwRingMetrics) Setinput_bytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("input_bytes"))
	return nil
}

// Setoutput_bytes sets cunter in shared memory
func (mtr *AccelHwRingMetrics) Setoutput_bytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("output_bytes"))
	return nil
}

// Setsoft_resets sets cunter in shared memory
func (mtr *AccelHwRingMetrics) Setsoft_resets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("soft_resets"))
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

	buf, _ := json.Marshal(key)
	mtr, err := it.iter.Find(string(buf))

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

	buf, _ := json.Marshal(key)
	mtr := it.iter.Create(string(buf), tmtr.Size())

	tmtr = &AccelHwRingMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *AccelHwRingMetricsIterator) Delete(key AccelHwRingKey) error {

	buf, _ := json.Marshal(key)
	return it.iter.Delete(string(buf))

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
