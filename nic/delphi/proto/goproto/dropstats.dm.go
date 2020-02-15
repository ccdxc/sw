// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/tsdb/metrics"
)

type DropMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DropMalformedPkt metrics.Counter

	DropParserIcrcError metrics.Counter

	DropParseLenError metrics.Counter

	DropHardwareError metrics.Counter

	DropInputMapping metrics.Counter

	DropInputMappingDejavu metrics.Counter

	DropMultiDestNotPinnedUplink metrics.Counter

	DropFlowHit metrics.Counter

	DropFlowMiss metrics.Counter

	DropNacl metrics.Counter

	DropIpsg metrics.Counter

	DropIpNormalization metrics.Counter

	DropTcpNormalization metrics.Counter

	DropTcpRstWithInvalidAckNum metrics.Counter

	DropTcpNonSynFirstPkt metrics.Counter

	DropIcmpNormalization metrics.Counter

	DropInputPropertiesMiss metrics.Counter

	DropTcpOutOfWindow metrics.Counter

	DropTcpSplitHandshake metrics.Counter

	DropTcpWinZeroDrop metrics.Counter

	DropTcpDataAfterFin metrics.Counter

	DropTcpNonRstPktAfterRst metrics.Counter

	DropTcpInvalidResponderFirstPkt metrics.Counter

	DropTcpUnexpectedPkt metrics.Counter

	DropSrcLifMismatch metrics.Counter

	DropVfIpLabelMismatch metrics.Counter

	DropVfBadRrDstIp metrics.Counter

	DropIcmpFragPkt metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *DropMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *DropMetrics) Size() int {
	sz := 0

	sz += mtr.DropMalformedPkt.Size()

	sz += mtr.DropParserIcrcError.Size()

	sz += mtr.DropParseLenError.Size()

	sz += mtr.DropHardwareError.Size()

	sz += mtr.DropInputMapping.Size()

	sz += mtr.DropInputMappingDejavu.Size()

	sz += mtr.DropMultiDestNotPinnedUplink.Size()

	sz += mtr.DropFlowHit.Size()

	sz += mtr.DropFlowMiss.Size()

	sz += mtr.DropNacl.Size()

	sz += mtr.DropIpsg.Size()

	sz += mtr.DropIpNormalization.Size()

	sz += mtr.DropTcpNormalization.Size()

	sz += mtr.DropTcpRstWithInvalidAckNum.Size()

	sz += mtr.DropTcpNonSynFirstPkt.Size()

	sz += mtr.DropIcmpNormalization.Size()

	sz += mtr.DropInputPropertiesMiss.Size()

	sz += mtr.DropTcpOutOfWindow.Size()

	sz += mtr.DropTcpSplitHandshake.Size()

	sz += mtr.DropTcpWinZeroDrop.Size()

	sz += mtr.DropTcpDataAfterFin.Size()

	sz += mtr.DropTcpNonRstPktAfterRst.Size()

	sz += mtr.DropTcpInvalidResponderFirstPkt.Size()

	sz += mtr.DropTcpUnexpectedPkt.Size()

	sz += mtr.DropSrcLifMismatch.Size()

	sz += mtr.DropVfIpLabelMismatch.Size()

	sz += mtr.DropVfBadRrDstIp.Size()

	sz += mtr.DropIcmpFragPkt.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *DropMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.DropMalformedPkt = mtr.metrics.GetCounter(offset)
	offset += mtr.DropMalformedPkt.Size()

	mtr.DropParserIcrcError = mtr.metrics.GetCounter(offset)
	offset += mtr.DropParserIcrcError.Size()

	mtr.DropParseLenError = mtr.metrics.GetCounter(offset)
	offset += mtr.DropParseLenError.Size()

	mtr.DropHardwareError = mtr.metrics.GetCounter(offset)
	offset += mtr.DropHardwareError.Size()

	mtr.DropInputMapping = mtr.metrics.GetCounter(offset)
	offset += mtr.DropInputMapping.Size()

	mtr.DropInputMappingDejavu = mtr.metrics.GetCounter(offset)
	offset += mtr.DropInputMappingDejavu.Size()

	mtr.DropMultiDestNotPinnedUplink = mtr.metrics.GetCounter(offset)
	offset += mtr.DropMultiDestNotPinnedUplink.Size()

	mtr.DropFlowHit = mtr.metrics.GetCounter(offset)
	offset += mtr.DropFlowHit.Size()

	mtr.DropFlowMiss = mtr.metrics.GetCounter(offset)
	offset += mtr.DropFlowMiss.Size()

	mtr.DropNacl = mtr.metrics.GetCounter(offset)
	offset += mtr.DropNacl.Size()

	mtr.DropIpsg = mtr.metrics.GetCounter(offset)
	offset += mtr.DropIpsg.Size()

	mtr.DropIpNormalization = mtr.metrics.GetCounter(offset)
	offset += mtr.DropIpNormalization.Size()

	mtr.DropTcpNormalization = mtr.metrics.GetCounter(offset)
	offset += mtr.DropTcpNormalization.Size()

	mtr.DropTcpRstWithInvalidAckNum = mtr.metrics.GetCounter(offset)
	offset += mtr.DropTcpRstWithInvalidAckNum.Size()

	mtr.DropTcpNonSynFirstPkt = mtr.metrics.GetCounter(offset)
	offset += mtr.DropTcpNonSynFirstPkt.Size()

	mtr.DropIcmpNormalization = mtr.metrics.GetCounter(offset)
	offset += mtr.DropIcmpNormalization.Size()

	mtr.DropInputPropertiesMiss = mtr.metrics.GetCounter(offset)
	offset += mtr.DropInputPropertiesMiss.Size()

	mtr.DropTcpOutOfWindow = mtr.metrics.GetCounter(offset)
	offset += mtr.DropTcpOutOfWindow.Size()

	mtr.DropTcpSplitHandshake = mtr.metrics.GetCounter(offset)
	offset += mtr.DropTcpSplitHandshake.Size()

	mtr.DropTcpWinZeroDrop = mtr.metrics.GetCounter(offset)
	offset += mtr.DropTcpWinZeroDrop.Size()

	mtr.DropTcpDataAfterFin = mtr.metrics.GetCounter(offset)
	offset += mtr.DropTcpDataAfterFin.Size()

	mtr.DropTcpNonRstPktAfterRst = mtr.metrics.GetCounter(offset)
	offset += mtr.DropTcpNonRstPktAfterRst.Size()

	mtr.DropTcpInvalidResponderFirstPkt = mtr.metrics.GetCounter(offset)
	offset += mtr.DropTcpInvalidResponderFirstPkt.Size()

	mtr.DropTcpUnexpectedPkt = mtr.metrics.GetCounter(offset)
	offset += mtr.DropTcpUnexpectedPkt.Size()

	mtr.DropSrcLifMismatch = mtr.metrics.GetCounter(offset)
	offset += mtr.DropSrcLifMismatch.Size()

	mtr.DropVfIpLabelMismatch = mtr.metrics.GetCounter(offset)
	offset += mtr.DropVfIpLabelMismatch.Size()

	mtr.DropVfBadRrDstIp = mtr.metrics.GetCounter(offset)
	offset += mtr.DropVfBadRrDstIp.Size()

	mtr.DropIcmpFragPkt = mtr.metrics.GetCounter(offset)
	offset += mtr.DropIcmpFragPkt.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *DropMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "DropMalformedPkt" {
		return offset
	}
	offset += mtr.DropMalformedPkt.Size()

	if fldName == "DropParserIcrcError" {
		return offset
	}
	offset += mtr.DropParserIcrcError.Size()

	if fldName == "DropParseLenError" {
		return offset
	}
	offset += mtr.DropParseLenError.Size()

	if fldName == "DropHardwareError" {
		return offset
	}
	offset += mtr.DropHardwareError.Size()

	if fldName == "DropInputMapping" {
		return offset
	}
	offset += mtr.DropInputMapping.Size()

	if fldName == "DropInputMappingDejavu" {
		return offset
	}
	offset += mtr.DropInputMappingDejavu.Size()

	if fldName == "DropMultiDestNotPinnedUplink" {
		return offset
	}
	offset += mtr.DropMultiDestNotPinnedUplink.Size()

	if fldName == "DropFlowHit" {
		return offset
	}
	offset += mtr.DropFlowHit.Size()

	if fldName == "DropFlowMiss" {
		return offset
	}
	offset += mtr.DropFlowMiss.Size()

	if fldName == "DropNacl" {
		return offset
	}
	offset += mtr.DropNacl.Size()

	if fldName == "DropIpsg" {
		return offset
	}
	offset += mtr.DropIpsg.Size()

	if fldName == "DropIpNormalization" {
		return offset
	}
	offset += mtr.DropIpNormalization.Size()

	if fldName == "DropTcpNormalization" {
		return offset
	}
	offset += mtr.DropTcpNormalization.Size()

	if fldName == "DropTcpRstWithInvalidAckNum" {
		return offset
	}
	offset += mtr.DropTcpRstWithInvalidAckNum.Size()

	if fldName == "DropTcpNonSynFirstPkt" {
		return offset
	}
	offset += mtr.DropTcpNonSynFirstPkt.Size()

	if fldName == "DropIcmpNormalization" {
		return offset
	}
	offset += mtr.DropIcmpNormalization.Size()

	if fldName == "DropInputPropertiesMiss" {
		return offset
	}
	offset += mtr.DropInputPropertiesMiss.Size()

	if fldName == "DropTcpOutOfWindow" {
		return offset
	}
	offset += mtr.DropTcpOutOfWindow.Size()

	if fldName == "DropTcpSplitHandshake" {
		return offset
	}
	offset += mtr.DropTcpSplitHandshake.Size()

	if fldName == "DropTcpWinZeroDrop" {
		return offset
	}
	offset += mtr.DropTcpWinZeroDrop.Size()

	if fldName == "DropTcpDataAfterFin" {
		return offset
	}
	offset += mtr.DropTcpDataAfterFin.Size()

	if fldName == "DropTcpNonRstPktAfterRst" {
		return offset
	}
	offset += mtr.DropTcpNonRstPktAfterRst.Size()

	if fldName == "DropTcpInvalidResponderFirstPkt" {
		return offset
	}
	offset += mtr.DropTcpInvalidResponderFirstPkt.Size()

	if fldName == "DropTcpUnexpectedPkt" {
		return offset
	}
	offset += mtr.DropTcpUnexpectedPkt.Size()

	if fldName == "DropSrcLifMismatch" {
		return offset
	}
	offset += mtr.DropSrcLifMismatch.Size()

	if fldName == "DropVfIpLabelMismatch" {
		return offset
	}
	offset += mtr.DropVfIpLabelMismatch.Size()

	if fldName == "DropVfBadRrDstIp" {
		return offset
	}
	offset += mtr.DropVfBadRrDstIp.Size()

	if fldName == "DropIcmpFragPkt" {
		return offset
	}
	offset += mtr.DropIcmpFragPkt.Size()

	return offset
}

// SetDropMalformedPkt sets cunter in shared memory
func (mtr *DropMetrics) SetDropMalformedPkt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropMalformedPkt"))
	return nil
}

// SetDropParserIcrcError sets cunter in shared memory
func (mtr *DropMetrics) SetDropParserIcrcError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropParserIcrcError"))
	return nil
}

// SetDropParseLenError sets cunter in shared memory
func (mtr *DropMetrics) SetDropParseLenError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropParseLenError"))
	return nil
}

// SetDropHardwareError sets cunter in shared memory
func (mtr *DropMetrics) SetDropHardwareError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropHardwareError"))
	return nil
}

// SetDropInputMapping sets cunter in shared memory
func (mtr *DropMetrics) SetDropInputMapping(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropInputMapping"))
	return nil
}

// SetDropInputMappingDejavu sets cunter in shared memory
func (mtr *DropMetrics) SetDropInputMappingDejavu(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropInputMappingDejavu"))
	return nil
}

// SetDropMultiDestNotPinnedUplink sets cunter in shared memory
func (mtr *DropMetrics) SetDropMultiDestNotPinnedUplink(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropMultiDestNotPinnedUplink"))
	return nil
}

// SetDropFlowHit sets cunter in shared memory
func (mtr *DropMetrics) SetDropFlowHit(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropFlowHit"))
	return nil
}

// SetDropFlowMiss sets cunter in shared memory
func (mtr *DropMetrics) SetDropFlowMiss(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropFlowMiss"))
	return nil
}

// SetDropNacl sets cunter in shared memory
func (mtr *DropMetrics) SetDropNacl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropNacl"))
	return nil
}

// SetDropIpsg sets cunter in shared memory
func (mtr *DropMetrics) SetDropIpsg(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropIpsg"))
	return nil
}

// SetDropIpNormalization sets cunter in shared memory
func (mtr *DropMetrics) SetDropIpNormalization(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropIpNormalization"))
	return nil
}

// SetDropTcpNormalization sets cunter in shared memory
func (mtr *DropMetrics) SetDropTcpNormalization(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropTcpNormalization"))
	return nil
}

// SetDropTcpRstWithInvalidAckNum sets cunter in shared memory
func (mtr *DropMetrics) SetDropTcpRstWithInvalidAckNum(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropTcpRstWithInvalidAckNum"))
	return nil
}

// SetDropTcpNonSynFirstPkt sets cunter in shared memory
func (mtr *DropMetrics) SetDropTcpNonSynFirstPkt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropTcpNonSynFirstPkt"))
	return nil
}

// SetDropIcmpNormalization sets cunter in shared memory
func (mtr *DropMetrics) SetDropIcmpNormalization(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropIcmpNormalization"))
	return nil
}

// SetDropInputPropertiesMiss sets cunter in shared memory
func (mtr *DropMetrics) SetDropInputPropertiesMiss(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropInputPropertiesMiss"))
	return nil
}

// SetDropTcpOutOfWindow sets cunter in shared memory
func (mtr *DropMetrics) SetDropTcpOutOfWindow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropTcpOutOfWindow"))
	return nil
}

// SetDropTcpSplitHandshake sets cunter in shared memory
func (mtr *DropMetrics) SetDropTcpSplitHandshake(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropTcpSplitHandshake"))
	return nil
}

// SetDropTcpWinZeroDrop sets cunter in shared memory
func (mtr *DropMetrics) SetDropTcpWinZeroDrop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropTcpWinZeroDrop"))
	return nil
}

// SetDropTcpDataAfterFin sets cunter in shared memory
func (mtr *DropMetrics) SetDropTcpDataAfterFin(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropTcpDataAfterFin"))
	return nil
}

// SetDropTcpNonRstPktAfterRst sets cunter in shared memory
func (mtr *DropMetrics) SetDropTcpNonRstPktAfterRst(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropTcpNonRstPktAfterRst"))
	return nil
}

// SetDropTcpInvalidResponderFirstPkt sets cunter in shared memory
func (mtr *DropMetrics) SetDropTcpInvalidResponderFirstPkt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropTcpInvalidResponderFirstPkt"))
	return nil
}

// SetDropTcpUnexpectedPkt sets cunter in shared memory
func (mtr *DropMetrics) SetDropTcpUnexpectedPkt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropTcpUnexpectedPkt"))
	return nil
}

// SetDropSrcLifMismatch sets cunter in shared memory
func (mtr *DropMetrics) SetDropSrcLifMismatch(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropSrcLifMismatch"))
	return nil
}

// SetDropVfIpLabelMismatch sets cunter in shared memory
func (mtr *DropMetrics) SetDropVfIpLabelMismatch(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropVfIpLabelMismatch"))
	return nil
}

// SetDropVfBadRrDstIp sets cunter in shared memory
func (mtr *DropMetrics) SetDropVfBadRrDstIp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropVfBadRrDstIp"))
	return nil
}

// SetDropIcmpFragPkt sets cunter in shared memory
func (mtr *DropMetrics) SetDropIcmpFragPkt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropIcmpFragPkt"))
	return nil
}

// DropMetricsIterator is the iterator object
type DropMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *DropMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *DropMetricsIterator) Next() *DropMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &DropMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *DropMetricsIterator) Find(key uint64) (*DropMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &DropMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *DropMetricsIterator) Create(key uint64) (*DropMetrics, error) {
	tmtr := &DropMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &DropMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *DropMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *DropMetricsIterator) Free() {
	it.iter.Free()
}

// NewDropMetricsIterator returns an iterator
func NewDropMetricsIterator() (*DropMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("DropMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &DropMetricsIterator{iter: iter}, nil
}

type EgressDropMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DropOutputMapping metrics.Counter

	DropPruneSrcPort metrics.Counter

	DropMirror metrics.Counter

	DropPolicer metrics.Counter

	DropCopp metrics.Counter

	DropChecksumErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *EgressDropMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *EgressDropMetrics) Size() int {
	sz := 0

	sz += mtr.DropOutputMapping.Size()

	sz += mtr.DropPruneSrcPort.Size()

	sz += mtr.DropMirror.Size()

	sz += mtr.DropPolicer.Size()

	sz += mtr.DropCopp.Size()

	sz += mtr.DropChecksumErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *EgressDropMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.DropOutputMapping = mtr.metrics.GetCounter(offset)
	offset += mtr.DropOutputMapping.Size()

	mtr.DropPruneSrcPort = mtr.metrics.GetCounter(offset)
	offset += mtr.DropPruneSrcPort.Size()

	mtr.DropMirror = mtr.metrics.GetCounter(offset)
	offset += mtr.DropMirror.Size()

	mtr.DropPolicer = mtr.metrics.GetCounter(offset)
	offset += mtr.DropPolicer.Size()

	mtr.DropCopp = mtr.metrics.GetCounter(offset)
	offset += mtr.DropCopp.Size()

	mtr.DropChecksumErr = mtr.metrics.GetCounter(offset)
	offset += mtr.DropChecksumErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *EgressDropMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "DropOutputMapping" {
		return offset
	}
	offset += mtr.DropOutputMapping.Size()

	if fldName == "DropPruneSrcPort" {
		return offset
	}
	offset += mtr.DropPruneSrcPort.Size()

	if fldName == "DropMirror" {
		return offset
	}
	offset += mtr.DropMirror.Size()

	if fldName == "DropPolicer" {
		return offset
	}
	offset += mtr.DropPolicer.Size()

	if fldName == "DropCopp" {
		return offset
	}
	offset += mtr.DropCopp.Size()

	if fldName == "DropChecksumErr" {
		return offset
	}
	offset += mtr.DropChecksumErr.Size()

	return offset
}

// SetDropOutputMapping sets cunter in shared memory
func (mtr *EgressDropMetrics) SetDropOutputMapping(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropOutputMapping"))
	return nil
}

// SetDropPruneSrcPort sets cunter in shared memory
func (mtr *EgressDropMetrics) SetDropPruneSrcPort(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropPruneSrcPort"))
	return nil
}

// SetDropMirror sets cunter in shared memory
func (mtr *EgressDropMetrics) SetDropMirror(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropMirror"))
	return nil
}

// SetDropPolicer sets cunter in shared memory
func (mtr *EgressDropMetrics) SetDropPolicer(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropPolicer"))
	return nil
}

// SetDropCopp sets cunter in shared memory
func (mtr *EgressDropMetrics) SetDropCopp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropCopp"))
	return nil
}

// SetDropChecksumErr sets cunter in shared memory
func (mtr *EgressDropMetrics) SetDropChecksumErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DropChecksumErr"))
	return nil
}

// EgressDropMetricsIterator is the iterator object
type EgressDropMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *EgressDropMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *EgressDropMetricsIterator) Next() *EgressDropMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &EgressDropMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *EgressDropMetricsIterator) Find(key uint64) (*EgressDropMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &EgressDropMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *EgressDropMetricsIterator) Create(key uint64) (*EgressDropMetrics, error) {
	tmtr := &EgressDropMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &EgressDropMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *EgressDropMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *EgressDropMetricsIterator) Free() {
	it.iter.Free()
}

// NewEgressDropMetricsIterator returns an iterator
func NewEgressDropMetricsIterator() (*EgressDropMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("EgressDropMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &EgressDropMetricsIterator{iter: iter}, nil
}
