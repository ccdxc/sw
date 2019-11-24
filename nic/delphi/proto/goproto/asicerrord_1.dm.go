// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/tsdb/metrics"
)

type Ptptptdintgrp1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RcvPhvDmaPtr metrics.Counter

	RcvPhvAddr metrics.Counter

	RcvExceed_16Byte metrics.Counter

	RcvPhvNotSop metrics.Counter

	RcvPktOrderFfFull metrics.Counter

	RcvPendPhvMoreThan_2 metrics.Counter

	RcvPendPhvLess metrics.Counter

	RcvCmdNopEop metrics.Counter

	RcvCmdOutNotSop metrics.Counter

	RcvPkt2MemSeen metrics.Counter

	RcvSkipSeen metrics.Counter

	RcvPhvEopNoCmdEop metrics.Counter

	RcvM2MDstNotSeen metrics.Counter

	RcvM2MSrcNotSeen metrics.Counter

	RdreqInvalidCmdSeen metrics.Counter

	RdreqMem2MemPsizeZero metrics.Counter

	RdreqM2MPhv2MemExceed_16Byte metrics.Counter

	RdreqPhv2MemFenceExceed_16Byte metrics.Counter

	RdreqMem2PktPsizeZero metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ptptptdintgrp1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ptptptdintgrp1Metrics) Size() int {
	sz := 0

	sz += mtr.RcvPhvDmaPtr.Size()

	sz += mtr.RcvPhvAddr.Size()

	sz += mtr.RcvExceed_16Byte.Size()

	sz += mtr.RcvPhvNotSop.Size()

	sz += mtr.RcvPktOrderFfFull.Size()

	sz += mtr.RcvPendPhvMoreThan_2.Size()

	sz += mtr.RcvPendPhvLess.Size()

	sz += mtr.RcvCmdNopEop.Size()

	sz += mtr.RcvCmdOutNotSop.Size()

	sz += mtr.RcvPkt2MemSeen.Size()

	sz += mtr.RcvSkipSeen.Size()

	sz += mtr.RcvPhvEopNoCmdEop.Size()

	sz += mtr.RcvM2MDstNotSeen.Size()

	sz += mtr.RcvM2MSrcNotSeen.Size()

	sz += mtr.RdreqInvalidCmdSeen.Size()

	sz += mtr.RdreqMem2MemPsizeZero.Size()

	sz += mtr.RdreqM2MPhv2MemExceed_16Byte.Size()

	sz += mtr.RdreqPhv2MemFenceExceed_16Byte.Size()

	sz += mtr.RdreqMem2PktPsizeZero.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ptptptdintgrp1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RcvPhvDmaPtr = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPhvDmaPtr.Size()

	mtr.RcvPhvAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPhvAddr.Size()

	mtr.RcvExceed_16Byte = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvExceed_16Byte.Size()

	mtr.RcvPhvNotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPhvNotSop.Size()

	mtr.RcvPktOrderFfFull = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPktOrderFfFull.Size()

	mtr.RcvPendPhvMoreThan_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPendPhvMoreThan_2.Size()

	mtr.RcvPendPhvLess = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPendPhvLess.Size()

	mtr.RcvCmdNopEop = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvCmdNopEop.Size()

	mtr.RcvCmdOutNotSop = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvCmdOutNotSop.Size()

	mtr.RcvPkt2MemSeen = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPkt2MemSeen.Size()

	mtr.RcvSkipSeen = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvSkipSeen.Size()

	mtr.RcvPhvEopNoCmdEop = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvPhvEopNoCmdEop.Size()

	mtr.RcvM2MDstNotSeen = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvM2MDstNotSeen.Size()

	mtr.RcvM2MSrcNotSeen = mtr.metrics.GetCounter(offset)
	offset += mtr.RcvM2MSrcNotSeen.Size()

	mtr.RdreqInvalidCmdSeen = mtr.metrics.GetCounter(offset)
	offset += mtr.RdreqInvalidCmdSeen.Size()

	mtr.RdreqMem2MemPsizeZero = mtr.metrics.GetCounter(offset)
	offset += mtr.RdreqMem2MemPsizeZero.Size()

	mtr.RdreqM2MPhv2MemExceed_16Byte = mtr.metrics.GetCounter(offset)
	offset += mtr.RdreqM2MPhv2MemExceed_16Byte.Size()

	mtr.RdreqPhv2MemFenceExceed_16Byte = mtr.metrics.GetCounter(offset)
	offset += mtr.RdreqPhv2MemFenceExceed_16Byte.Size()

	mtr.RdreqMem2PktPsizeZero = mtr.metrics.GetCounter(offset)
	offset += mtr.RdreqMem2PktPsizeZero.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ptptptdintgrp1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RcvPhvDmaPtr" {
		return offset
	}
	offset += mtr.RcvPhvDmaPtr.Size()

	if fldName == "RcvPhvAddr" {
		return offset
	}
	offset += mtr.RcvPhvAddr.Size()

	if fldName == "RcvExceed_16Byte" {
		return offset
	}
	offset += mtr.RcvExceed_16Byte.Size()

	if fldName == "RcvPhvNotSop" {
		return offset
	}
	offset += mtr.RcvPhvNotSop.Size()

	if fldName == "RcvPktOrderFfFull" {
		return offset
	}
	offset += mtr.RcvPktOrderFfFull.Size()

	if fldName == "RcvPendPhvMoreThan_2" {
		return offset
	}
	offset += mtr.RcvPendPhvMoreThan_2.Size()

	if fldName == "RcvPendPhvLess" {
		return offset
	}
	offset += mtr.RcvPendPhvLess.Size()

	if fldName == "RcvCmdNopEop" {
		return offset
	}
	offset += mtr.RcvCmdNopEop.Size()

	if fldName == "RcvCmdOutNotSop" {
		return offset
	}
	offset += mtr.RcvCmdOutNotSop.Size()

	if fldName == "RcvPkt2MemSeen" {
		return offset
	}
	offset += mtr.RcvPkt2MemSeen.Size()

	if fldName == "RcvSkipSeen" {
		return offset
	}
	offset += mtr.RcvSkipSeen.Size()

	if fldName == "RcvPhvEopNoCmdEop" {
		return offset
	}
	offset += mtr.RcvPhvEopNoCmdEop.Size()

	if fldName == "RcvM2MDstNotSeen" {
		return offset
	}
	offset += mtr.RcvM2MDstNotSeen.Size()

	if fldName == "RcvM2MSrcNotSeen" {
		return offset
	}
	offset += mtr.RcvM2MSrcNotSeen.Size()

	if fldName == "RdreqInvalidCmdSeen" {
		return offset
	}
	offset += mtr.RdreqInvalidCmdSeen.Size()

	if fldName == "RdreqMem2MemPsizeZero" {
		return offset
	}
	offset += mtr.RdreqMem2MemPsizeZero.Size()

	if fldName == "RdreqM2MPhv2MemExceed_16Byte" {
		return offset
	}
	offset += mtr.RdreqM2MPhv2MemExceed_16Byte.Size()

	if fldName == "RdreqPhv2MemFenceExceed_16Byte" {
		return offset
	}
	offset += mtr.RdreqPhv2MemFenceExceed_16Byte.Size()

	if fldName == "RdreqMem2PktPsizeZero" {
		return offset
	}
	offset += mtr.RdreqMem2PktPsizeZero.Size()

	return offset
}

// SetRcvPhvDmaPtr sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvPhvDmaPtr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPhvDmaPtr"))
	return nil
}

// SetRcvPhvAddr sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvPhvAddr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPhvAddr"))
	return nil
}

// SetRcvExceed_16Byte sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvExceed_16Byte(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvExceed_16Byte"))
	return nil
}

// SetRcvPhvNotSop sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvPhvNotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPhvNotSop"))
	return nil
}

// SetRcvPktOrderFfFull sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvPktOrderFfFull(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPktOrderFfFull"))
	return nil
}

// SetRcvPendPhvMoreThan_2 sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvPendPhvMoreThan_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPendPhvMoreThan_2"))
	return nil
}

// SetRcvPendPhvLess sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvPendPhvLess(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPendPhvLess"))
	return nil
}

// SetRcvCmdNopEop sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvCmdNopEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvCmdNopEop"))
	return nil
}

// SetRcvCmdOutNotSop sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvCmdOutNotSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvCmdOutNotSop"))
	return nil
}

// SetRcvPkt2MemSeen sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvPkt2MemSeen(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPkt2MemSeen"))
	return nil
}

// SetRcvSkipSeen sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvSkipSeen(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvSkipSeen"))
	return nil
}

// SetRcvPhvEopNoCmdEop sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvPhvEopNoCmdEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvPhvEopNoCmdEop"))
	return nil
}

// SetRcvM2MDstNotSeen sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvM2MDstNotSeen(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvM2MDstNotSeen"))
	return nil
}

// SetRcvM2MSrcNotSeen sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRcvM2MSrcNotSeen(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RcvM2MSrcNotSeen"))
	return nil
}

// SetRdreqInvalidCmdSeen sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRdreqInvalidCmdSeen(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdreqInvalidCmdSeen"))
	return nil
}

// SetRdreqMem2MemPsizeZero sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRdreqMem2MemPsizeZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdreqMem2MemPsizeZero"))
	return nil
}

// SetRdreqM2MPhv2MemExceed_16Byte sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRdreqM2MPhv2MemExceed_16Byte(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdreqM2MPhv2MemExceed_16Byte"))
	return nil
}

// SetRdreqPhv2MemFenceExceed_16Byte sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRdreqPhv2MemFenceExceed_16Byte(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdreqPhv2MemFenceExceed_16Byte"))
	return nil
}

// SetRdreqMem2PktPsizeZero sets cunter in shared memory
func (mtr *Ptptptdintgrp1Metrics) SetRdreqMem2PktPsizeZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdreqMem2PktPsizeZero"))
	return nil
}

// Ptptptdintgrp1MetricsIterator is the iterator object
type Ptptptdintgrp1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ptptptdintgrp1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ptptptdintgrp1MetricsIterator) Next() *Ptptptdintgrp1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ptptptdintgrp1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ptptptdintgrp1MetricsIterator) Find(key uint64) (*Ptptptdintgrp1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ptptptdintgrp1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ptptptdintgrp1MetricsIterator) Create(key uint64) (*Ptptptdintgrp1Metrics, error) {
	tmtr := &Ptptptdintgrp1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ptptptdintgrp1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ptptptdintgrp1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ptptptdintgrp1MetricsIterator) Free() {
	it.iter.Free()
}

// NewPtptptdintgrp1MetricsIterator returns an iterator
func NewPtptptdintgrp1MetricsIterator() (*Ptptptdintgrp1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ptptptdintgrp1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ptptptdintgrp1MetricsIterator{iter: iter}, nil
}

type Ptptptdintgrp2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	WrInvalidCmd metrics.Counter

	WrPendSz metrics.Counter

	WrAxiRdResp metrics.Counter

	PktErrEopForced metrics.Counter

	PktNoEopErrSeen metrics.Counter

	PktCmdEopNoData metrics.Counter

	RdrspAxi metrics.Counter

	WrrspAxi metrics.Counter

	SpuriousRdResp metrics.Counter

	SpuriousWrResp metrics.Counter

	WrreqInfoFirstMissing metrics.Counter

	WrreqNumBytesZero metrics.Counter

	WrreqNumBytesMoreThan_64 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Ptptptdintgrp2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Ptptptdintgrp2Metrics) Size() int {
	sz := 0

	sz += mtr.WrInvalidCmd.Size()

	sz += mtr.WrPendSz.Size()

	sz += mtr.WrAxiRdResp.Size()

	sz += mtr.PktErrEopForced.Size()

	sz += mtr.PktNoEopErrSeen.Size()

	sz += mtr.PktCmdEopNoData.Size()

	sz += mtr.RdrspAxi.Size()

	sz += mtr.WrrspAxi.Size()

	sz += mtr.SpuriousRdResp.Size()

	sz += mtr.SpuriousWrResp.Size()

	sz += mtr.WrreqInfoFirstMissing.Size()

	sz += mtr.WrreqNumBytesZero.Size()

	sz += mtr.WrreqNumBytesMoreThan_64.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Ptptptdintgrp2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.WrInvalidCmd = mtr.metrics.GetCounter(offset)
	offset += mtr.WrInvalidCmd.Size()

	mtr.WrPendSz = mtr.metrics.GetCounter(offset)
	offset += mtr.WrPendSz.Size()

	mtr.WrAxiRdResp = mtr.metrics.GetCounter(offset)
	offset += mtr.WrAxiRdResp.Size()

	mtr.PktErrEopForced = mtr.metrics.GetCounter(offset)
	offset += mtr.PktErrEopForced.Size()

	mtr.PktNoEopErrSeen = mtr.metrics.GetCounter(offset)
	offset += mtr.PktNoEopErrSeen.Size()

	mtr.PktCmdEopNoData = mtr.metrics.GetCounter(offset)
	offset += mtr.PktCmdEopNoData.Size()

	mtr.RdrspAxi = mtr.metrics.GetCounter(offset)
	offset += mtr.RdrspAxi.Size()

	mtr.WrrspAxi = mtr.metrics.GetCounter(offset)
	offset += mtr.WrrspAxi.Size()

	mtr.SpuriousRdResp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousRdResp.Size()

	mtr.SpuriousWrResp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousWrResp.Size()

	mtr.WrreqInfoFirstMissing = mtr.metrics.GetCounter(offset)
	offset += mtr.WrreqInfoFirstMissing.Size()

	mtr.WrreqNumBytesZero = mtr.metrics.GetCounter(offset)
	offset += mtr.WrreqNumBytesZero.Size()

	mtr.WrreqNumBytesMoreThan_64 = mtr.metrics.GetCounter(offset)
	offset += mtr.WrreqNumBytesMoreThan_64.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Ptptptdintgrp2Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "WrInvalidCmd" {
		return offset
	}
	offset += mtr.WrInvalidCmd.Size()

	if fldName == "WrPendSz" {
		return offset
	}
	offset += mtr.WrPendSz.Size()

	if fldName == "WrAxiRdResp" {
		return offset
	}
	offset += mtr.WrAxiRdResp.Size()

	if fldName == "PktErrEopForced" {
		return offset
	}
	offset += mtr.PktErrEopForced.Size()

	if fldName == "PktNoEopErrSeen" {
		return offset
	}
	offset += mtr.PktNoEopErrSeen.Size()

	if fldName == "PktCmdEopNoData" {
		return offset
	}
	offset += mtr.PktCmdEopNoData.Size()

	if fldName == "RdrspAxi" {
		return offset
	}
	offset += mtr.RdrspAxi.Size()

	if fldName == "WrrspAxi" {
		return offset
	}
	offset += mtr.WrrspAxi.Size()

	if fldName == "SpuriousRdResp" {
		return offset
	}
	offset += mtr.SpuriousRdResp.Size()

	if fldName == "SpuriousWrResp" {
		return offset
	}
	offset += mtr.SpuriousWrResp.Size()

	if fldName == "WrreqInfoFirstMissing" {
		return offset
	}
	offset += mtr.WrreqInfoFirstMissing.Size()

	if fldName == "WrreqNumBytesZero" {
		return offset
	}
	offset += mtr.WrreqNumBytesZero.Size()

	if fldName == "WrreqNumBytesMoreThan_64" {
		return offset
	}
	offset += mtr.WrreqNumBytesMoreThan_64.Size()

	return offset
}

// SetWrInvalidCmd sets cunter in shared memory
func (mtr *Ptptptdintgrp2Metrics) SetWrInvalidCmd(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrInvalidCmd"))
	return nil
}

// SetWrPendSz sets cunter in shared memory
func (mtr *Ptptptdintgrp2Metrics) SetWrPendSz(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrPendSz"))
	return nil
}

// SetWrAxiRdResp sets cunter in shared memory
func (mtr *Ptptptdintgrp2Metrics) SetWrAxiRdResp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrAxiRdResp"))
	return nil
}

// SetPktErrEopForced sets cunter in shared memory
func (mtr *Ptptptdintgrp2Metrics) SetPktErrEopForced(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktErrEopForced"))
	return nil
}

// SetPktNoEopErrSeen sets cunter in shared memory
func (mtr *Ptptptdintgrp2Metrics) SetPktNoEopErrSeen(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktNoEopErrSeen"))
	return nil
}

// SetPktCmdEopNoData sets cunter in shared memory
func (mtr *Ptptptdintgrp2Metrics) SetPktCmdEopNoData(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktCmdEopNoData"))
	return nil
}

// SetRdrspAxi sets cunter in shared memory
func (mtr *Ptptptdintgrp2Metrics) SetRdrspAxi(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdrspAxi"))
	return nil
}

// SetWrrspAxi sets cunter in shared memory
func (mtr *Ptptptdintgrp2Metrics) SetWrrspAxi(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrrspAxi"))
	return nil
}

// SetSpuriousRdResp sets cunter in shared memory
func (mtr *Ptptptdintgrp2Metrics) SetSpuriousRdResp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousRdResp"))
	return nil
}

// SetSpuriousWrResp sets cunter in shared memory
func (mtr *Ptptptdintgrp2Metrics) SetSpuriousWrResp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousWrResp"))
	return nil
}

// SetWrreqInfoFirstMissing sets cunter in shared memory
func (mtr *Ptptptdintgrp2Metrics) SetWrreqInfoFirstMissing(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrreqInfoFirstMissing"))
	return nil
}

// SetWrreqNumBytesZero sets cunter in shared memory
func (mtr *Ptptptdintgrp2Metrics) SetWrreqNumBytesZero(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrreqNumBytesZero"))
	return nil
}

// SetWrreqNumBytesMoreThan_64 sets cunter in shared memory
func (mtr *Ptptptdintgrp2Metrics) SetWrreqNumBytesMoreThan_64(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrreqNumBytesMoreThan_64"))
	return nil
}

// Ptptptdintgrp2MetricsIterator is the iterator object
type Ptptptdintgrp2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Ptptptdintgrp2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Ptptptdintgrp2MetricsIterator) Next() *Ptptptdintgrp2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Ptptptdintgrp2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Ptptptdintgrp2MetricsIterator) Find(key uint64) (*Ptptptdintgrp2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Ptptptdintgrp2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Ptptptdintgrp2MetricsIterator) Create(key uint64) (*Ptptptdintgrp2Metrics, error) {
	tmtr := &Ptptptdintgrp2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Ptptptdintgrp2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Ptptptdintgrp2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Ptptptdintgrp2MetricsIterator) Free() {
	it.iter.Free()
}

// NewPtptptdintgrp2MetricsIterator returns an iterator
func NewPtptptdintgrp2MetricsIterator() (*Ptptptdintgrp2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Ptptptdintgrp2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Ptptptdintgrp2MetricsIterator{iter: iter}, nil
}

type PtptptdintintfMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PbErr metrics.Counter

	PbSopErr metrics.Counter

	PbEopErr metrics.Counter

	NpvResubErr metrics.Counter

	NpvResubSopErr metrics.Counter

	NpvResubEopErr metrics.Counter

	MaErr metrics.Counter

	MaSopErr metrics.Counter

	MaEopErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PtptptdintintfMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PtptptdintintfMetrics) Size() int {
	sz := 0

	sz += mtr.PbErr.Size()

	sz += mtr.PbSopErr.Size()

	sz += mtr.PbEopErr.Size()

	sz += mtr.NpvResubErr.Size()

	sz += mtr.NpvResubSopErr.Size()

	sz += mtr.NpvResubEopErr.Size()

	sz += mtr.MaErr.Size()

	sz += mtr.MaSopErr.Size()

	sz += mtr.MaEopErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PtptptdintintfMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PbErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbErr.Size()

	mtr.PbSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbSopErr.Size()

	mtr.PbEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.PbEopErr.Size()

	mtr.NpvResubErr = mtr.metrics.GetCounter(offset)
	offset += mtr.NpvResubErr.Size()

	mtr.NpvResubSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.NpvResubSopErr.Size()

	mtr.NpvResubEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.NpvResubEopErr.Size()

	mtr.MaErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaErr.Size()

	mtr.MaSopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaSopErr.Size()

	mtr.MaEopErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MaEopErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PtptptdintintfMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PbErr" {
		return offset
	}
	offset += mtr.PbErr.Size()

	if fldName == "PbSopErr" {
		return offset
	}
	offset += mtr.PbSopErr.Size()

	if fldName == "PbEopErr" {
		return offset
	}
	offset += mtr.PbEopErr.Size()

	if fldName == "NpvResubErr" {
		return offset
	}
	offset += mtr.NpvResubErr.Size()

	if fldName == "NpvResubSopErr" {
		return offset
	}
	offset += mtr.NpvResubSopErr.Size()

	if fldName == "NpvResubEopErr" {
		return offset
	}
	offset += mtr.NpvResubEopErr.Size()

	if fldName == "MaErr" {
		return offset
	}
	offset += mtr.MaErr.Size()

	if fldName == "MaSopErr" {
		return offset
	}
	offset += mtr.MaSopErr.Size()

	if fldName == "MaEopErr" {
		return offset
	}
	offset += mtr.MaEopErr.Size()

	return offset
}

// SetPbErr sets cunter in shared memory
func (mtr *PtptptdintintfMetrics) SetPbErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbErr"))
	return nil
}

// SetPbSopErr sets cunter in shared memory
func (mtr *PtptptdintintfMetrics) SetPbSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbSopErr"))
	return nil
}

// SetPbEopErr sets cunter in shared memory
func (mtr *PtptptdintintfMetrics) SetPbEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PbEopErr"))
	return nil
}

// SetNpvResubErr sets cunter in shared memory
func (mtr *PtptptdintintfMetrics) SetNpvResubErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NpvResubErr"))
	return nil
}

// SetNpvResubSopErr sets cunter in shared memory
func (mtr *PtptptdintintfMetrics) SetNpvResubSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NpvResubSopErr"))
	return nil
}

// SetNpvResubEopErr sets cunter in shared memory
func (mtr *PtptptdintintfMetrics) SetNpvResubEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NpvResubEopErr"))
	return nil
}

// SetMaErr sets cunter in shared memory
func (mtr *PtptptdintintfMetrics) SetMaErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaErr"))
	return nil
}

// SetMaSopErr sets cunter in shared memory
func (mtr *PtptptdintintfMetrics) SetMaSopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaSopErr"))
	return nil
}

// SetMaEopErr sets cunter in shared memory
func (mtr *PtptptdintintfMetrics) SetMaEopErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaEopErr"))
	return nil
}

// PtptptdintintfMetricsIterator is the iterator object
type PtptptdintintfMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PtptptdintintfMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PtptptdintintfMetricsIterator) Next() *PtptptdintintfMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PtptptdintintfMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PtptptdintintfMetricsIterator) Find(key uint64) (*PtptptdintintfMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PtptptdintintfMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PtptptdintintfMetricsIterator) Create(key uint64) (*PtptptdintintfMetrics, error) {
	tmtr := &PtptptdintintfMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PtptptdintintfMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PtptptdintintfMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PtptptdintintfMetricsIterator) Free() {
	it.iter.Free()
}

// NewPtptptdintintfMetricsIterator returns an iterator
func NewPtptptdintintfMetricsIterator() (*PtptptdintintfMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PtptptdintintfMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PtptptdintintfMetricsIterator{iter: iter}, nil
}

type Pcrte0interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrte0interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrte0interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrte0interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrte0interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pcrte0interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pcrte0interrMetricsIterator is the iterator object
type Pcrte0interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrte0interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrte0interrMetricsIterator) Next() *Pcrte0interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrte0interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrte0interrMetricsIterator) Find(key uint64) (*Pcrte0interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrte0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrte0interrMetricsIterator) Create(key uint64) (*Pcrte0interrMetrics, error) {
	tmtr := &Pcrte0interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrte0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrte0interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrte0interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrte0interrMetricsIterator returns an iterator
func NewPcrte0interrMetricsIterator() (*Pcrte0interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrte0interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrte0interrMetricsIterator{iter: iter}, nil
}

type Pcrte1interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrte1interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrte1interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrte1interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrte1interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pcrte1interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pcrte1interrMetricsIterator is the iterator object
type Pcrte1interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrte1interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrte1interrMetricsIterator) Next() *Pcrte1interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrte1interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrte1interrMetricsIterator) Find(key uint64) (*Pcrte1interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrte1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrte1interrMetricsIterator) Create(key uint64) (*Pcrte1interrMetrics, error) {
	tmtr := &Pcrte1interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrte1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrte1interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrte1interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrte1interrMetricsIterator returns an iterator
func NewPcrte1interrMetricsIterator() (*Pcrte1interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrte1interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrte1interrMetricsIterator{iter: iter}, nil
}

type Pcrte2interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrte2interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrte2interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrte2interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrte2interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pcrte2interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pcrte2interrMetricsIterator is the iterator object
type Pcrte2interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrte2interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrte2interrMetricsIterator) Next() *Pcrte2interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrte2interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrte2interrMetricsIterator) Find(key uint64) (*Pcrte2interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrte2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrte2interrMetricsIterator) Create(key uint64) (*Pcrte2interrMetrics, error) {
	tmtr := &Pcrte2interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrte2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrte2interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrte2interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrte2interrMetricsIterator returns an iterator
func NewPcrte2interrMetricsIterator() (*Pcrte2interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrte2interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrte2interrMetricsIterator{iter: iter}, nil
}

type Pcrte3interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrte3interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrte3interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrte3interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrte3interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pcrte3interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pcrte3interrMetricsIterator is the iterator object
type Pcrte3interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrte3interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrte3interrMetricsIterator) Next() *Pcrte3interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrte3interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrte3interrMetricsIterator) Find(key uint64) (*Pcrte3interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrte3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrte3interrMetricsIterator) Create(key uint64) (*Pcrte3interrMetrics, error) {
	tmtr := &Pcrte3interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrte3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrte3interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrte3interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrte3interrMetricsIterator returns an iterator
func NewPcrte3interrMetricsIterator() (*Pcrte3interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrte3interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrte3interrMetricsIterator{iter: iter}, nil
}

type Pcrte4interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrte4interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrte4interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrte4interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrte4interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pcrte4interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pcrte4interrMetricsIterator is the iterator object
type Pcrte4interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrte4interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrte4interrMetricsIterator) Next() *Pcrte4interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrte4interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrte4interrMetricsIterator) Find(key uint64) (*Pcrte4interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrte4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrte4interrMetricsIterator) Create(key uint64) (*Pcrte4interrMetrics, error) {
	tmtr := &Pcrte4interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrte4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrte4interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrte4interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrte4interrMetricsIterator returns an iterator
func NewPcrte4interrMetricsIterator() (*Pcrte4interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrte4interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrte4interrMetricsIterator{iter: iter}, nil
}

type Pcrte5interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrte5interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrte5interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrte5interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrte5interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pcrte5interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pcrte5interrMetricsIterator is the iterator object
type Pcrte5interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrte5interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrte5interrMetricsIterator) Next() *Pcrte5interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrte5interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrte5interrMetricsIterator) Find(key uint64) (*Pcrte5interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrte5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrte5interrMetricsIterator) Create(key uint64) (*Pcrte5interrMetrics, error) {
	tmtr := &Pcrte5interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrte5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrte5interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrte5interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrte5interrMetricsIterator returns an iterator
func NewPcrte5interrMetricsIterator() (*Pcrte5interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrte5interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrte5interrMetricsIterator{iter: iter}, nil
}

type Pcrte6interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrte6interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrte6interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrte6interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrte6interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pcrte6interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pcrte6interrMetricsIterator is the iterator object
type Pcrte6interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrte6interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrte6interrMetricsIterator) Next() *Pcrte6interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrte6interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrte6interrMetricsIterator) Find(key uint64) (*Pcrte6interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrte6interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrte6interrMetricsIterator) Create(key uint64) (*Pcrte6interrMetrics, error) {
	tmtr := &Pcrte6interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrte6interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrte6interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrte6interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrte6interrMetricsIterator returns an iterator
func NewPcrte6interrMetricsIterator() (*Pcrte6interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrte6interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrte6interrMetricsIterator{iter: iter}, nil
}

type Pcrte7interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrte7interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrte7interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrte7interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrte7interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pcrte7interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pcrte7interrMetricsIterator is the iterator object
type Pcrte7interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrte7interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrte7interrMetricsIterator) Next() *Pcrte7interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrte7interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrte7interrMetricsIterator) Find(key uint64) (*Pcrte7interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrte7interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrte7interrMetricsIterator) Create(key uint64) (*Pcrte7interrMetrics, error) {
	tmtr := &Pcrte7interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrte7interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrte7interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrte7interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrte7interrMetricsIterator returns an iterator
func NewPcrte7interrMetricsIterator() (*Pcrte7interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrte7interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrte7interrMetricsIterator{iter: iter}, nil
}

type Pcrmpu0interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu0interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu0interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu0interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu0interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pcrmpu0interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pcrmpu0interrMetricsIterator is the iterator object
type Pcrmpu0interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu0interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu0interrMetricsIterator) Next() *Pcrmpu0interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu0interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu0interrMetricsIterator) Find(key uint64) (*Pcrmpu0interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu0interrMetricsIterator) Create(key uint64) (*Pcrmpu0interrMetrics, error) {
	tmtr := &Pcrmpu0interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu0interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu0interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu0interrMetricsIterator returns an iterator
func NewPcrmpu0interrMetricsIterator() (*Pcrmpu0interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu0interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu0interrMetricsIterator{iter: iter}, nil
}

type Pcrmpu0intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu0intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu0intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu0intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu0intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pcrmpu0intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pcrmpu0intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pcrmpu0intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pcrmpu0intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pcrmpu0intinfoMetricsIterator is the iterator object
type Pcrmpu0intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu0intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu0intinfoMetricsIterator) Next() *Pcrmpu0intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu0intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu0intinfoMetricsIterator) Find(key uint64) (*Pcrmpu0intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu0intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu0intinfoMetricsIterator) Create(key uint64) (*Pcrmpu0intinfoMetrics, error) {
	tmtr := &Pcrmpu0intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu0intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu0intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu0intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu0intinfoMetricsIterator returns an iterator
func NewPcrmpu0intinfoMetricsIterator() (*Pcrmpu0intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu0intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu0intinfoMetricsIterator{iter: iter}, nil
}

type Pcrmpu1interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu1interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu1interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu1interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu1interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pcrmpu1interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pcrmpu1interrMetricsIterator is the iterator object
type Pcrmpu1interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu1interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu1interrMetricsIterator) Next() *Pcrmpu1interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu1interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu1interrMetricsIterator) Find(key uint64) (*Pcrmpu1interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu1interrMetricsIterator) Create(key uint64) (*Pcrmpu1interrMetrics, error) {
	tmtr := &Pcrmpu1interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu1interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu1interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu1interrMetricsIterator returns an iterator
func NewPcrmpu1interrMetricsIterator() (*Pcrmpu1interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu1interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu1interrMetricsIterator{iter: iter}, nil
}

type Pcrmpu1intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu1intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu1intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu1intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu1intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pcrmpu1intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pcrmpu1intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pcrmpu1intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pcrmpu1intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pcrmpu1intinfoMetricsIterator is the iterator object
type Pcrmpu1intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu1intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu1intinfoMetricsIterator) Next() *Pcrmpu1intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu1intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu1intinfoMetricsIterator) Find(key uint64) (*Pcrmpu1intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu1intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu1intinfoMetricsIterator) Create(key uint64) (*Pcrmpu1intinfoMetrics, error) {
	tmtr := &Pcrmpu1intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu1intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu1intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu1intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu1intinfoMetricsIterator returns an iterator
func NewPcrmpu1intinfoMetricsIterator() (*Pcrmpu1intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu1intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu1intinfoMetricsIterator{iter: iter}, nil
}

type Pcrmpu2interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu2interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu2interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu2interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu2interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pcrmpu2interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pcrmpu2interrMetricsIterator is the iterator object
type Pcrmpu2interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu2interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu2interrMetricsIterator) Next() *Pcrmpu2interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu2interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu2interrMetricsIterator) Find(key uint64) (*Pcrmpu2interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu2interrMetricsIterator) Create(key uint64) (*Pcrmpu2interrMetrics, error) {
	tmtr := &Pcrmpu2interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu2interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu2interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu2interrMetricsIterator returns an iterator
func NewPcrmpu2interrMetricsIterator() (*Pcrmpu2interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu2interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu2interrMetricsIterator{iter: iter}, nil
}

type Pcrmpu2intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu2intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu2intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu2intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu2intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pcrmpu2intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pcrmpu2intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pcrmpu2intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pcrmpu2intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pcrmpu2intinfoMetricsIterator is the iterator object
type Pcrmpu2intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu2intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu2intinfoMetricsIterator) Next() *Pcrmpu2intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu2intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu2intinfoMetricsIterator) Find(key uint64) (*Pcrmpu2intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu2intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu2intinfoMetricsIterator) Create(key uint64) (*Pcrmpu2intinfoMetrics, error) {
	tmtr := &Pcrmpu2intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu2intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu2intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu2intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu2intinfoMetricsIterator returns an iterator
func NewPcrmpu2intinfoMetricsIterator() (*Pcrmpu2intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu2intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu2intinfoMetricsIterator{iter: iter}, nil
}

type Pcrmpu3interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu3interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu3interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu3interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu3interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pcrmpu3interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pcrmpu3interrMetricsIterator is the iterator object
type Pcrmpu3interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu3interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu3interrMetricsIterator) Next() *Pcrmpu3interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu3interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu3interrMetricsIterator) Find(key uint64) (*Pcrmpu3interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu3interrMetricsIterator) Create(key uint64) (*Pcrmpu3interrMetrics, error) {
	tmtr := &Pcrmpu3interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu3interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu3interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu3interrMetricsIterator returns an iterator
func NewPcrmpu3interrMetricsIterator() (*Pcrmpu3interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu3interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu3interrMetricsIterator{iter: iter}, nil
}

type Pcrmpu3intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu3intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu3intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu3intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu3intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pcrmpu3intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pcrmpu3intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pcrmpu3intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pcrmpu3intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pcrmpu3intinfoMetricsIterator is the iterator object
type Pcrmpu3intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu3intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu3intinfoMetricsIterator) Next() *Pcrmpu3intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu3intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu3intinfoMetricsIterator) Find(key uint64) (*Pcrmpu3intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu3intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu3intinfoMetricsIterator) Create(key uint64) (*Pcrmpu3intinfoMetrics, error) {
	tmtr := &Pcrmpu3intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu3intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu3intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu3intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu3intinfoMetricsIterator returns an iterator
func NewPcrmpu3intinfoMetricsIterator() (*Pcrmpu3intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu3intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu3intinfoMetricsIterator{iter: iter}, nil
}

type Pcrmpu4interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu4interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu4interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu4interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu4interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pcrmpu4interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pcrmpu4interrMetricsIterator is the iterator object
type Pcrmpu4interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu4interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu4interrMetricsIterator) Next() *Pcrmpu4interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu4interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu4interrMetricsIterator) Find(key uint64) (*Pcrmpu4interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu4interrMetricsIterator) Create(key uint64) (*Pcrmpu4interrMetrics, error) {
	tmtr := &Pcrmpu4interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu4interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu4interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu4interrMetricsIterator returns an iterator
func NewPcrmpu4interrMetricsIterator() (*Pcrmpu4interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu4interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu4interrMetricsIterator{iter: iter}, nil
}

type Pcrmpu4intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu4intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu4intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu4intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu4intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pcrmpu4intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pcrmpu4intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pcrmpu4intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pcrmpu4intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pcrmpu4intinfoMetricsIterator is the iterator object
type Pcrmpu4intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu4intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu4intinfoMetricsIterator) Next() *Pcrmpu4intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu4intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu4intinfoMetricsIterator) Find(key uint64) (*Pcrmpu4intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu4intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu4intinfoMetricsIterator) Create(key uint64) (*Pcrmpu4intinfoMetrics, error) {
	tmtr := &Pcrmpu4intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu4intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu4intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu4intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu4intinfoMetricsIterator returns an iterator
func NewPcrmpu4intinfoMetricsIterator() (*Pcrmpu4intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu4intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu4intinfoMetricsIterator{iter: iter}, nil
}

type Pcrmpu5interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu5interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu5interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu5interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu5interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pcrmpu5interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pcrmpu5interrMetricsIterator is the iterator object
type Pcrmpu5interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu5interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu5interrMetricsIterator) Next() *Pcrmpu5interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu5interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu5interrMetricsIterator) Find(key uint64) (*Pcrmpu5interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu5interrMetricsIterator) Create(key uint64) (*Pcrmpu5interrMetrics, error) {
	tmtr := &Pcrmpu5interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu5interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu5interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu5interrMetricsIterator returns an iterator
func NewPcrmpu5interrMetricsIterator() (*Pcrmpu5interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu5interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu5interrMetricsIterator{iter: iter}, nil
}

type Pcrmpu5intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu5intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu5intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu5intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu5intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pcrmpu5intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pcrmpu5intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pcrmpu5intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pcrmpu5intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pcrmpu5intinfoMetricsIterator is the iterator object
type Pcrmpu5intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu5intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu5intinfoMetricsIterator) Next() *Pcrmpu5intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu5intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu5intinfoMetricsIterator) Find(key uint64) (*Pcrmpu5intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu5intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu5intinfoMetricsIterator) Create(key uint64) (*Pcrmpu5intinfoMetrics, error) {
	tmtr := &Pcrmpu5intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu5intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu5intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu5intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu5intinfoMetricsIterator returns an iterator
func NewPcrmpu5intinfoMetricsIterator() (*Pcrmpu5intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu5intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu5intinfoMetricsIterator{iter: iter}, nil
}

type Pcrmpu6interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu6interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu6interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu6interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu6interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pcrmpu6interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pcrmpu6interrMetricsIterator is the iterator object
type Pcrmpu6interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu6interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu6interrMetricsIterator) Next() *Pcrmpu6interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu6interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu6interrMetricsIterator) Find(key uint64) (*Pcrmpu6interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu6interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu6interrMetricsIterator) Create(key uint64) (*Pcrmpu6interrMetrics, error) {
	tmtr := &Pcrmpu6interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu6interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu6interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu6interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu6interrMetricsIterator returns an iterator
func NewPcrmpu6interrMetricsIterator() (*Pcrmpu6interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu6interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu6interrMetricsIterator{iter: iter}, nil
}

type Pcrmpu6intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu6intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu6intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu6intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu6intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pcrmpu6intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pcrmpu6intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pcrmpu6intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pcrmpu6intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pcrmpu6intinfoMetricsIterator is the iterator object
type Pcrmpu6intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu6intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu6intinfoMetricsIterator) Next() *Pcrmpu6intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu6intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu6intinfoMetricsIterator) Find(key uint64) (*Pcrmpu6intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu6intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu6intinfoMetricsIterator) Create(key uint64) (*Pcrmpu6intinfoMetrics, error) {
	tmtr := &Pcrmpu6intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu6intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu6intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu6intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu6intinfoMetricsIterator returns an iterator
func NewPcrmpu6intinfoMetricsIterator() (*Pcrmpu6intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu6intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu6intinfoMetricsIterator{iter: iter}, nil
}

type Pcrmpu7interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu7interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu7interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu7interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu7interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pcrmpu7interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pcrmpu7interrMetricsIterator is the iterator object
type Pcrmpu7interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu7interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu7interrMetricsIterator) Next() *Pcrmpu7interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu7interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu7interrMetricsIterator) Find(key uint64) (*Pcrmpu7interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu7interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu7interrMetricsIterator) Create(key uint64) (*Pcrmpu7interrMetrics, error) {
	tmtr := &Pcrmpu7interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu7interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu7interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu7interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu7interrMetricsIterator returns an iterator
func NewPcrmpu7interrMetricsIterator() (*Pcrmpu7interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu7interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu7interrMetricsIterator{iter: iter}, nil
}

type Pcrmpu7intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pcrmpu7intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pcrmpu7intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pcrmpu7intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pcrmpu7intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pcrmpu7intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pcrmpu7intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pcrmpu7intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pcrmpu7intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pcrmpu7intinfoMetricsIterator is the iterator object
type Pcrmpu7intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pcrmpu7intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pcrmpu7intinfoMetricsIterator) Next() *Pcrmpu7intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pcrmpu7intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pcrmpu7intinfoMetricsIterator) Find(key uint64) (*Pcrmpu7intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pcrmpu7intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pcrmpu7intinfoMetricsIterator) Create(key uint64) (*Pcrmpu7intinfoMetrics, error) {
	tmtr := &Pcrmpu7intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pcrmpu7intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pcrmpu7intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pcrmpu7intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPcrmpu7intinfoMetricsIterator returns an iterator
func NewPcrmpu7intinfoMetricsIterator() (*Pcrmpu7intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pcrmpu7intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pcrmpu7intinfoMetricsIterator{iter: iter}, nil
}

type TxstxsintschMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SchRidEmpty metrics.Counter

	SchWrTxfifoOvf metrics.Counter

	SchRdTxfifoOvf metrics.Counter

	SchStateFifoOvf metrics.Counter

	SchHbmBypOvf metrics.Counter

	SchHbmBypWtagWrap metrics.Counter

	SchRlidOvfl metrics.Counter

	SchRlidUnfl metrics.Counter

	SchNullLif metrics.Counter

	SchLifSgMismatch metrics.Counter

	SchAclrHbmLnRollovr metrics.Counter

	SchBidErr metrics.Counter

	SchRrespErr metrics.Counter

	SchRidErr metrics.Counter

	SchDrbCntOvfl metrics.Counter

	SchDrbCntUnfl metrics.Counter

	SchTxdmaMsgOvfl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *TxstxsintschMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *TxstxsintschMetrics) Size() int {
	sz := 0

	sz += mtr.SchRidEmpty.Size()

	sz += mtr.SchWrTxfifoOvf.Size()

	sz += mtr.SchRdTxfifoOvf.Size()

	sz += mtr.SchStateFifoOvf.Size()

	sz += mtr.SchHbmBypOvf.Size()

	sz += mtr.SchHbmBypWtagWrap.Size()

	sz += mtr.SchRlidOvfl.Size()

	sz += mtr.SchRlidUnfl.Size()

	sz += mtr.SchNullLif.Size()

	sz += mtr.SchLifSgMismatch.Size()

	sz += mtr.SchAclrHbmLnRollovr.Size()

	sz += mtr.SchBidErr.Size()

	sz += mtr.SchRrespErr.Size()

	sz += mtr.SchRidErr.Size()

	sz += mtr.SchDrbCntOvfl.Size()

	sz += mtr.SchDrbCntUnfl.Size()

	sz += mtr.SchTxdmaMsgOvfl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *TxstxsintschMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SchRidEmpty = mtr.metrics.GetCounter(offset)
	offset += mtr.SchRidEmpty.Size()

	mtr.SchWrTxfifoOvf = mtr.metrics.GetCounter(offset)
	offset += mtr.SchWrTxfifoOvf.Size()

	mtr.SchRdTxfifoOvf = mtr.metrics.GetCounter(offset)
	offset += mtr.SchRdTxfifoOvf.Size()

	mtr.SchStateFifoOvf = mtr.metrics.GetCounter(offset)
	offset += mtr.SchStateFifoOvf.Size()

	mtr.SchHbmBypOvf = mtr.metrics.GetCounter(offset)
	offset += mtr.SchHbmBypOvf.Size()

	mtr.SchHbmBypWtagWrap = mtr.metrics.GetCounter(offset)
	offset += mtr.SchHbmBypWtagWrap.Size()

	mtr.SchRlidOvfl = mtr.metrics.GetCounter(offset)
	offset += mtr.SchRlidOvfl.Size()

	mtr.SchRlidUnfl = mtr.metrics.GetCounter(offset)
	offset += mtr.SchRlidUnfl.Size()

	mtr.SchNullLif = mtr.metrics.GetCounter(offset)
	offset += mtr.SchNullLif.Size()

	mtr.SchLifSgMismatch = mtr.metrics.GetCounter(offset)
	offset += mtr.SchLifSgMismatch.Size()

	mtr.SchAclrHbmLnRollovr = mtr.metrics.GetCounter(offset)
	offset += mtr.SchAclrHbmLnRollovr.Size()

	mtr.SchBidErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SchBidErr.Size()

	mtr.SchRrespErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SchRrespErr.Size()

	mtr.SchRidErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SchRidErr.Size()

	mtr.SchDrbCntOvfl = mtr.metrics.GetCounter(offset)
	offset += mtr.SchDrbCntOvfl.Size()

	mtr.SchDrbCntUnfl = mtr.metrics.GetCounter(offset)
	offset += mtr.SchDrbCntUnfl.Size()

	mtr.SchTxdmaMsgOvfl = mtr.metrics.GetCounter(offset)
	offset += mtr.SchTxdmaMsgOvfl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *TxstxsintschMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SchRidEmpty" {
		return offset
	}
	offset += mtr.SchRidEmpty.Size()

	if fldName == "SchWrTxfifoOvf" {
		return offset
	}
	offset += mtr.SchWrTxfifoOvf.Size()

	if fldName == "SchRdTxfifoOvf" {
		return offset
	}
	offset += mtr.SchRdTxfifoOvf.Size()

	if fldName == "SchStateFifoOvf" {
		return offset
	}
	offset += mtr.SchStateFifoOvf.Size()

	if fldName == "SchHbmBypOvf" {
		return offset
	}
	offset += mtr.SchHbmBypOvf.Size()

	if fldName == "SchHbmBypWtagWrap" {
		return offset
	}
	offset += mtr.SchHbmBypWtagWrap.Size()

	if fldName == "SchRlidOvfl" {
		return offset
	}
	offset += mtr.SchRlidOvfl.Size()

	if fldName == "SchRlidUnfl" {
		return offset
	}
	offset += mtr.SchRlidUnfl.Size()

	if fldName == "SchNullLif" {
		return offset
	}
	offset += mtr.SchNullLif.Size()

	if fldName == "SchLifSgMismatch" {
		return offset
	}
	offset += mtr.SchLifSgMismatch.Size()

	if fldName == "SchAclrHbmLnRollovr" {
		return offset
	}
	offset += mtr.SchAclrHbmLnRollovr.Size()

	if fldName == "SchBidErr" {
		return offset
	}
	offset += mtr.SchBidErr.Size()

	if fldName == "SchRrespErr" {
		return offset
	}
	offset += mtr.SchRrespErr.Size()

	if fldName == "SchRidErr" {
		return offset
	}
	offset += mtr.SchRidErr.Size()

	if fldName == "SchDrbCntOvfl" {
		return offset
	}
	offset += mtr.SchDrbCntOvfl.Size()

	if fldName == "SchDrbCntUnfl" {
		return offset
	}
	offset += mtr.SchDrbCntUnfl.Size()

	if fldName == "SchTxdmaMsgOvfl" {
		return offset
	}
	offset += mtr.SchTxdmaMsgOvfl.Size()

	return offset
}

// SetSchRidEmpty sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchRidEmpty(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchRidEmpty"))
	return nil
}

// SetSchWrTxfifoOvf sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchWrTxfifoOvf(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchWrTxfifoOvf"))
	return nil
}

// SetSchRdTxfifoOvf sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchRdTxfifoOvf(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchRdTxfifoOvf"))
	return nil
}

// SetSchStateFifoOvf sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchStateFifoOvf(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchStateFifoOvf"))
	return nil
}

// SetSchHbmBypOvf sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchHbmBypOvf(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchHbmBypOvf"))
	return nil
}

// SetSchHbmBypWtagWrap sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchHbmBypWtagWrap(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchHbmBypWtagWrap"))
	return nil
}

// SetSchRlidOvfl sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchRlidOvfl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchRlidOvfl"))
	return nil
}

// SetSchRlidUnfl sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchRlidUnfl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchRlidUnfl"))
	return nil
}

// SetSchNullLif sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchNullLif(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchNullLif"))
	return nil
}

// SetSchLifSgMismatch sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchLifSgMismatch(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchLifSgMismatch"))
	return nil
}

// SetSchAclrHbmLnRollovr sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchAclrHbmLnRollovr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchAclrHbmLnRollovr"))
	return nil
}

// SetSchBidErr sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchBidErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchBidErr"))
	return nil
}

// SetSchRrespErr sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchRrespErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchRrespErr"))
	return nil
}

// SetSchRidErr sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchRidErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchRidErr"))
	return nil
}

// SetSchDrbCntOvfl sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchDrbCntOvfl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchDrbCntOvfl"))
	return nil
}

// SetSchDrbCntUnfl sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchDrbCntUnfl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchDrbCntUnfl"))
	return nil
}

// SetSchTxdmaMsgOvfl sets cunter in shared memory
func (mtr *TxstxsintschMetrics) SetSchTxdmaMsgOvfl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchTxdmaMsgOvfl"))
	return nil
}

// TxstxsintschMetricsIterator is the iterator object
type TxstxsintschMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *TxstxsintschMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *TxstxsintschMetricsIterator) Next() *TxstxsintschMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &TxstxsintschMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *TxstxsintschMetricsIterator) Find(key uint64) (*TxstxsintschMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &TxstxsintschMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *TxstxsintschMetricsIterator) Create(key uint64) (*TxstxsintschMetrics, error) {
	tmtr := &TxstxsintschMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &TxstxsintschMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *TxstxsintschMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *TxstxsintschMetricsIterator) Free() {
	it.iter.Free()
}

// NewTxstxsintschMetricsIterator returns an iterator
func NewTxstxsintschMetricsIterator() (*TxstxsintschMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("TxstxsintschMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &TxstxsintschMetricsIterator{iter: iter}, nil
}

type TxstxsinttmrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	TmrRidEmpty metrics.Counter

	TmrWrTxfifoOvf metrics.Counter

	TmrRdTxfifoOvf metrics.Counter

	TmrStateFifoOvf metrics.Counter

	TmrHbmBypOvf metrics.Counter

	TmrHbmBypWtagWrap metrics.Counter

	FtmrCtimeWrap metrics.Counter

	StmrCtimeWrap metrics.Counter

	FtmrFifoOvf metrics.Counter

	StmrFifoOvf metrics.Counter

	TmrRejctDrbOvf metrics.Counter

	TmrBidErr metrics.Counter

	TmrRidErr metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *TxstxsinttmrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *TxstxsinttmrMetrics) Size() int {
	sz := 0

	sz += mtr.TmrRidEmpty.Size()

	sz += mtr.TmrWrTxfifoOvf.Size()

	sz += mtr.TmrRdTxfifoOvf.Size()

	sz += mtr.TmrStateFifoOvf.Size()

	sz += mtr.TmrHbmBypOvf.Size()

	sz += mtr.TmrHbmBypWtagWrap.Size()

	sz += mtr.FtmrCtimeWrap.Size()

	sz += mtr.StmrCtimeWrap.Size()

	sz += mtr.FtmrFifoOvf.Size()

	sz += mtr.StmrFifoOvf.Size()

	sz += mtr.TmrRejctDrbOvf.Size()

	sz += mtr.TmrBidErr.Size()

	sz += mtr.TmrRidErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *TxstxsinttmrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.TmrRidEmpty = mtr.metrics.GetCounter(offset)
	offset += mtr.TmrRidEmpty.Size()

	mtr.TmrWrTxfifoOvf = mtr.metrics.GetCounter(offset)
	offset += mtr.TmrWrTxfifoOvf.Size()

	mtr.TmrRdTxfifoOvf = mtr.metrics.GetCounter(offset)
	offset += mtr.TmrRdTxfifoOvf.Size()

	mtr.TmrStateFifoOvf = mtr.metrics.GetCounter(offset)
	offset += mtr.TmrStateFifoOvf.Size()

	mtr.TmrHbmBypOvf = mtr.metrics.GetCounter(offset)
	offset += mtr.TmrHbmBypOvf.Size()

	mtr.TmrHbmBypWtagWrap = mtr.metrics.GetCounter(offset)
	offset += mtr.TmrHbmBypWtagWrap.Size()

	mtr.FtmrCtimeWrap = mtr.metrics.GetCounter(offset)
	offset += mtr.FtmrCtimeWrap.Size()

	mtr.StmrCtimeWrap = mtr.metrics.GetCounter(offset)
	offset += mtr.StmrCtimeWrap.Size()

	mtr.FtmrFifoOvf = mtr.metrics.GetCounter(offset)
	offset += mtr.FtmrFifoOvf.Size()

	mtr.StmrFifoOvf = mtr.metrics.GetCounter(offset)
	offset += mtr.StmrFifoOvf.Size()

	mtr.TmrRejctDrbOvf = mtr.metrics.GetCounter(offset)
	offset += mtr.TmrRejctDrbOvf.Size()

	mtr.TmrBidErr = mtr.metrics.GetCounter(offset)
	offset += mtr.TmrBidErr.Size()

	mtr.TmrRidErr = mtr.metrics.GetCounter(offset)
	offset += mtr.TmrRidErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *TxstxsinttmrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "TmrRidEmpty" {
		return offset
	}
	offset += mtr.TmrRidEmpty.Size()

	if fldName == "TmrWrTxfifoOvf" {
		return offset
	}
	offset += mtr.TmrWrTxfifoOvf.Size()

	if fldName == "TmrRdTxfifoOvf" {
		return offset
	}
	offset += mtr.TmrRdTxfifoOvf.Size()

	if fldName == "TmrStateFifoOvf" {
		return offset
	}
	offset += mtr.TmrStateFifoOvf.Size()

	if fldName == "TmrHbmBypOvf" {
		return offset
	}
	offset += mtr.TmrHbmBypOvf.Size()

	if fldName == "TmrHbmBypWtagWrap" {
		return offset
	}
	offset += mtr.TmrHbmBypWtagWrap.Size()

	if fldName == "FtmrCtimeWrap" {
		return offset
	}
	offset += mtr.FtmrCtimeWrap.Size()

	if fldName == "StmrCtimeWrap" {
		return offset
	}
	offset += mtr.StmrCtimeWrap.Size()

	if fldName == "FtmrFifoOvf" {
		return offset
	}
	offset += mtr.FtmrFifoOvf.Size()

	if fldName == "StmrFifoOvf" {
		return offset
	}
	offset += mtr.StmrFifoOvf.Size()

	if fldName == "TmrRejctDrbOvf" {
		return offset
	}
	offset += mtr.TmrRejctDrbOvf.Size()

	if fldName == "TmrBidErr" {
		return offset
	}
	offset += mtr.TmrBidErr.Size()

	if fldName == "TmrRidErr" {
		return offset
	}
	offset += mtr.TmrRidErr.Size()

	return offset
}

// SetTmrRidEmpty sets cunter in shared memory
func (mtr *TxstxsinttmrMetrics) SetTmrRidEmpty(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TmrRidEmpty"))
	return nil
}

// SetTmrWrTxfifoOvf sets cunter in shared memory
func (mtr *TxstxsinttmrMetrics) SetTmrWrTxfifoOvf(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TmrWrTxfifoOvf"))
	return nil
}

// SetTmrRdTxfifoOvf sets cunter in shared memory
func (mtr *TxstxsinttmrMetrics) SetTmrRdTxfifoOvf(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TmrRdTxfifoOvf"))
	return nil
}

// SetTmrStateFifoOvf sets cunter in shared memory
func (mtr *TxstxsinttmrMetrics) SetTmrStateFifoOvf(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TmrStateFifoOvf"))
	return nil
}

// SetTmrHbmBypOvf sets cunter in shared memory
func (mtr *TxstxsinttmrMetrics) SetTmrHbmBypOvf(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TmrHbmBypOvf"))
	return nil
}

// SetTmrHbmBypWtagWrap sets cunter in shared memory
func (mtr *TxstxsinttmrMetrics) SetTmrHbmBypWtagWrap(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TmrHbmBypWtagWrap"))
	return nil
}

// SetFtmrCtimeWrap sets cunter in shared memory
func (mtr *TxstxsinttmrMetrics) SetFtmrCtimeWrap(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FtmrCtimeWrap"))
	return nil
}

// SetStmrCtimeWrap sets cunter in shared memory
func (mtr *TxstxsinttmrMetrics) SetStmrCtimeWrap(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StmrCtimeWrap"))
	return nil
}

// SetFtmrFifoOvf sets cunter in shared memory
func (mtr *TxstxsinttmrMetrics) SetFtmrFifoOvf(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FtmrFifoOvf"))
	return nil
}

// SetStmrFifoOvf sets cunter in shared memory
func (mtr *TxstxsinttmrMetrics) SetStmrFifoOvf(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("StmrFifoOvf"))
	return nil
}

// SetTmrRejctDrbOvf sets cunter in shared memory
func (mtr *TxstxsinttmrMetrics) SetTmrRejctDrbOvf(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TmrRejctDrbOvf"))
	return nil
}

// SetTmrBidErr sets cunter in shared memory
func (mtr *TxstxsinttmrMetrics) SetTmrBidErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TmrBidErr"))
	return nil
}

// SetTmrRidErr sets cunter in shared memory
func (mtr *TxstxsinttmrMetrics) SetTmrRidErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TmrRidErr"))
	return nil
}

// TxstxsinttmrMetricsIterator is the iterator object
type TxstxsinttmrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *TxstxsinttmrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *TxstxsinttmrMetricsIterator) Next() *TxstxsinttmrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &TxstxsinttmrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *TxstxsinttmrMetricsIterator) Find(key uint64) (*TxstxsinttmrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &TxstxsinttmrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *TxstxsinttmrMetricsIterator) Create(key uint64) (*TxstxsinttmrMetrics, error) {
	tmtr := &TxstxsinttmrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &TxstxsinttmrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *TxstxsinttmrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *TxstxsinttmrMetricsIterator) Free() {
	it.iter.Free()
}

// NewTxstxsinttmrMetricsIterator returns an iterator
func NewTxstxsinttmrMetricsIterator() (*TxstxsinttmrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("TxstxsinttmrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &TxstxsinttmrMetricsIterator{iter: iter}, nil
}

type TxstxsintsramseccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	TmrCntUncorrectable metrics.Counter

	TmrCntCorrectable metrics.Counter

	SchLifMapUncorrectable metrics.Counter

	SchLifMapCorrectable metrics.Counter

	SchRlidMapUncorrectable metrics.Counter

	SchRlidMapCorrectable metrics.Counter

	SchGrpUncorrectable metrics.Counter

	SchGrpCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *TxstxsintsramseccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *TxstxsintsramseccMetrics) Size() int {
	sz := 0

	sz += mtr.TmrCntUncorrectable.Size()

	sz += mtr.TmrCntCorrectable.Size()

	sz += mtr.SchLifMapUncorrectable.Size()

	sz += mtr.SchLifMapCorrectable.Size()

	sz += mtr.SchRlidMapUncorrectable.Size()

	sz += mtr.SchRlidMapCorrectable.Size()

	sz += mtr.SchGrpUncorrectable.Size()

	sz += mtr.SchGrpCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *TxstxsintsramseccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.TmrCntUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.TmrCntUncorrectable.Size()

	mtr.TmrCntCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.TmrCntCorrectable.Size()

	mtr.SchLifMapUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SchLifMapUncorrectable.Size()

	mtr.SchLifMapCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SchLifMapCorrectable.Size()

	mtr.SchRlidMapUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SchRlidMapUncorrectable.Size()

	mtr.SchRlidMapCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SchRlidMapCorrectable.Size()

	mtr.SchGrpUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SchGrpUncorrectable.Size()

	mtr.SchGrpCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SchGrpCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *TxstxsintsramseccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "TmrCntUncorrectable" {
		return offset
	}
	offset += mtr.TmrCntUncorrectable.Size()

	if fldName == "TmrCntCorrectable" {
		return offset
	}
	offset += mtr.TmrCntCorrectable.Size()

	if fldName == "SchLifMapUncorrectable" {
		return offset
	}
	offset += mtr.SchLifMapUncorrectable.Size()

	if fldName == "SchLifMapCorrectable" {
		return offset
	}
	offset += mtr.SchLifMapCorrectable.Size()

	if fldName == "SchRlidMapUncorrectable" {
		return offset
	}
	offset += mtr.SchRlidMapUncorrectable.Size()

	if fldName == "SchRlidMapCorrectable" {
		return offset
	}
	offset += mtr.SchRlidMapCorrectable.Size()

	if fldName == "SchGrpUncorrectable" {
		return offset
	}
	offset += mtr.SchGrpUncorrectable.Size()

	if fldName == "SchGrpCorrectable" {
		return offset
	}
	offset += mtr.SchGrpCorrectable.Size()

	return offset
}

// SetTmrCntUncorrectable sets cunter in shared memory
func (mtr *TxstxsintsramseccMetrics) SetTmrCntUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TmrCntUncorrectable"))
	return nil
}

// SetTmrCntCorrectable sets cunter in shared memory
func (mtr *TxstxsintsramseccMetrics) SetTmrCntCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TmrCntCorrectable"))
	return nil
}

// SetSchLifMapUncorrectable sets cunter in shared memory
func (mtr *TxstxsintsramseccMetrics) SetSchLifMapUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchLifMapUncorrectable"))
	return nil
}

// SetSchLifMapCorrectable sets cunter in shared memory
func (mtr *TxstxsintsramseccMetrics) SetSchLifMapCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchLifMapCorrectable"))
	return nil
}

// SetSchRlidMapUncorrectable sets cunter in shared memory
func (mtr *TxstxsintsramseccMetrics) SetSchRlidMapUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchRlidMapUncorrectable"))
	return nil
}

// SetSchRlidMapCorrectable sets cunter in shared memory
func (mtr *TxstxsintsramseccMetrics) SetSchRlidMapCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchRlidMapCorrectable"))
	return nil
}

// SetSchGrpUncorrectable sets cunter in shared memory
func (mtr *TxstxsintsramseccMetrics) SetSchGrpUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchGrpUncorrectable"))
	return nil
}

// SetSchGrpCorrectable sets cunter in shared memory
func (mtr *TxstxsintsramseccMetrics) SetSchGrpCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SchGrpCorrectable"))
	return nil
}

// TxstxsintsramseccMetricsIterator is the iterator object
type TxstxsintsramseccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *TxstxsintsramseccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *TxstxsintsramseccMetricsIterator) Next() *TxstxsintsramseccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &TxstxsintsramseccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *TxstxsintsramseccMetricsIterator) Find(key uint64) (*TxstxsintsramseccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &TxstxsintsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *TxstxsintsramseccMetricsIterator) Create(key uint64) (*TxstxsintsramseccMetrics, error) {
	tmtr := &TxstxsintsramseccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &TxstxsintsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *TxstxsintsramseccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *TxstxsintsramseccMetricsIterator) Free() {
	it.iter.Free()
}

// NewTxstxsintsramseccMetricsIterator returns an iterator
func NewTxstxsintsramseccMetricsIterator() (*TxstxsintsramseccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("TxstxsintsramseccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &TxstxsintsramseccMetricsIterator{iter: iter}, nil
}

type Pctte0interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctte0interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctte0interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctte0interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctte0interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pctte0interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pctte0interrMetricsIterator is the iterator object
type Pctte0interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctte0interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctte0interrMetricsIterator) Next() *Pctte0interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctte0interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctte0interrMetricsIterator) Find(key uint64) (*Pctte0interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctte0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctte0interrMetricsIterator) Create(key uint64) (*Pctte0interrMetrics, error) {
	tmtr := &Pctte0interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctte0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctte0interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctte0interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctte0interrMetricsIterator returns an iterator
func NewPctte0interrMetricsIterator() (*Pctte0interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctte0interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctte0interrMetricsIterator{iter: iter}, nil
}

type Pctte1interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctte1interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctte1interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctte1interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctte1interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pctte1interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pctte1interrMetricsIterator is the iterator object
type Pctte1interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctte1interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctte1interrMetricsIterator) Next() *Pctte1interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctte1interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctte1interrMetricsIterator) Find(key uint64) (*Pctte1interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctte1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctte1interrMetricsIterator) Create(key uint64) (*Pctte1interrMetrics, error) {
	tmtr := &Pctte1interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctte1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctte1interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctte1interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctte1interrMetricsIterator returns an iterator
func NewPctte1interrMetricsIterator() (*Pctte1interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctte1interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctte1interrMetricsIterator{iter: iter}, nil
}

type Pctte2interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctte2interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctte2interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctte2interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctte2interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pctte2interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pctte2interrMetricsIterator is the iterator object
type Pctte2interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctte2interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctte2interrMetricsIterator) Next() *Pctte2interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctte2interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctte2interrMetricsIterator) Find(key uint64) (*Pctte2interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctte2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctte2interrMetricsIterator) Create(key uint64) (*Pctte2interrMetrics, error) {
	tmtr := &Pctte2interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctte2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctte2interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctte2interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctte2interrMetricsIterator returns an iterator
func NewPctte2interrMetricsIterator() (*Pctte2interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctte2interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctte2interrMetricsIterator{iter: iter}, nil
}

type Pctte3interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctte3interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctte3interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctte3interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctte3interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pctte3interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pctte3interrMetricsIterator is the iterator object
type Pctte3interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctte3interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctte3interrMetricsIterator) Next() *Pctte3interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctte3interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctte3interrMetricsIterator) Find(key uint64) (*Pctte3interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctte3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctte3interrMetricsIterator) Create(key uint64) (*Pctte3interrMetrics, error) {
	tmtr := &Pctte3interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctte3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctte3interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctte3interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctte3interrMetricsIterator returns an iterator
func NewPctte3interrMetricsIterator() (*Pctte3interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctte3interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctte3interrMetricsIterator{iter: iter}, nil
}

type Pctte4interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctte4interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctte4interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctte4interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctte4interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pctte4interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pctte4interrMetricsIterator is the iterator object
type Pctte4interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctte4interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctte4interrMetricsIterator) Next() *Pctte4interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctte4interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctte4interrMetricsIterator) Find(key uint64) (*Pctte4interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctte4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctte4interrMetricsIterator) Create(key uint64) (*Pctte4interrMetrics, error) {
	tmtr := &Pctte4interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctte4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctte4interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctte4interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctte4interrMetricsIterator returns an iterator
func NewPctte4interrMetricsIterator() (*Pctte4interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctte4interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctte4interrMetricsIterator{iter: iter}, nil
}

type Pctte5interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctte5interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctte5interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctte5interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctte5interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pctte5interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pctte5interrMetricsIterator is the iterator object
type Pctte5interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctte5interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctte5interrMetricsIterator) Next() *Pctte5interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctte5interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctte5interrMetricsIterator) Find(key uint64) (*Pctte5interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctte5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctte5interrMetricsIterator) Create(key uint64) (*Pctte5interrMetrics, error) {
	tmtr := &Pctte5interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctte5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctte5interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctte5interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctte5interrMetricsIterator returns an iterator
func NewPctte5interrMetricsIterator() (*Pctte5interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctte5interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctte5interrMetricsIterator{iter: iter}, nil
}

type Pctte6interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctte6interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctte6interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctte6interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctte6interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pctte6interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pctte6interrMetricsIterator is the iterator object
type Pctte6interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctte6interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctte6interrMetricsIterator) Next() *Pctte6interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctte6interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctte6interrMetricsIterator) Find(key uint64) (*Pctte6interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctte6interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctte6interrMetricsIterator) Create(key uint64) (*Pctte6interrMetrics, error) {
	tmtr := &Pctte6interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctte6interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctte6interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctte6interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctte6interrMetricsIterator returns an iterator
func NewPctte6interrMetricsIterator() (*Pctte6interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctte6interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctte6interrMetricsIterator{iter: iter}, nil
}

type Pctte7interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop metrics.Counter

	MissEop metrics.Counter

	PhvMaxSize metrics.Counter

	SpuriousAxiRsp metrics.Counter

	SpuriousTcamRsp metrics.Counter

	AxiRdrspErr metrics.Counter

	AxiBadRead metrics.Counter

	TcamReqIdxFifo metrics.Counter

	TcamRspIdxFifo metrics.Counter

	MpuReqIdxFifo metrics.Counter

	AxiReqIdxFifo metrics.Counter

	ProcTblVldWoProc metrics.Counter

	PendWoWb metrics.Counter

	Pend1WoPend0 metrics.Counter

	BothPendDown metrics.Counter

	PendWoProcDown metrics.Counter

	BothPendWentUp metrics.Counter

	LoadedButNoProc metrics.Counter

	LoadedButNoProcTblVld metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctte7interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctte7interrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctte7interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctte7interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetMissSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetMissEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetPhvMaxSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetSpuriousAxiRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetSpuriousTcamRsp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetAxiRdrspErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetAxiBadRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetTcamReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetTcamRspIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetMpuReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetAxiReqIdxFifo(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetProcTblVldWoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetPendWoWb(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetPend1WoPend0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetBothPendDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetPendWoProcDown(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetBothPendWentUp(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetLoadedButNoProc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *Pctte7interrMetrics) SetLoadedButNoProcTblVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// Pctte7interrMetricsIterator is the iterator object
type Pctte7interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctte7interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctte7interrMetricsIterator) Next() *Pctte7interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctte7interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctte7interrMetricsIterator) Find(key uint64) (*Pctte7interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctte7interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctte7interrMetricsIterator) Create(key uint64) (*Pctte7interrMetrics, error) {
	tmtr := &Pctte7interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctte7interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctte7interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctte7interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctte7interrMetricsIterator returns an iterator
func NewPctte7interrMetricsIterator() (*Pctte7interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctte7interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctte7interrMetricsIterator{iter: iter}, nil
}

type Pctmpu0interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu0interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu0interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu0interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu0interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pctmpu0interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pctmpu0interrMetricsIterator is the iterator object
type Pctmpu0interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu0interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu0interrMetricsIterator) Next() *Pctmpu0interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu0interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu0interrMetricsIterator) Find(key uint64) (*Pctmpu0interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu0interrMetricsIterator) Create(key uint64) (*Pctmpu0interrMetrics, error) {
	tmtr := &Pctmpu0interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu0interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu0interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu0interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu0interrMetricsIterator returns an iterator
func NewPctmpu0interrMetricsIterator() (*Pctmpu0interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu0interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu0interrMetricsIterator{iter: iter}, nil
}

type Pctmpu0intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu0intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu0intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu0intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu0intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pctmpu0intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pctmpu0intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pctmpu0intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pctmpu0intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pctmpu0intinfoMetricsIterator is the iterator object
type Pctmpu0intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu0intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu0intinfoMetricsIterator) Next() *Pctmpu0intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu0intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu0intinfoMetricsIterator) Find(key uint64) (*Pctmpu0intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu0intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu0intinfoMetricsIterator) Create(key uint64) (*Pctmpu0intinfoMetrics, error) {
	tmtr := &Pctmpu0intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu0intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu0intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu0intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu0intinfoMetricsIterator returns an iterator
func NewPctmpu0intinfoMetricsIterator() (*Pctmpu0intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu0intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu0intinfoMetricsIterator{iter: iter}, nil
}

type Pctmpu1interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu1interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu1interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu1interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu1interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pctmpu1interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pctmpu1interrMetricsIterator is the iterator object
type Pctmpu1interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu1interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu1interrMetricsIterator) Next() *Pctmpu1interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu1interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu1interrMetricsIterator) Find(key uint64) (*Pctmpu1interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu1interrMetricsIterator) Create(key uint64) (*Pctmpu1interrMetrics, error) {
	tmtr := &Pctmpu1interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu1interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu1interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu1interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu1interrMetricsIterator returns an iterator
func NewPctmpu1interrMetricsIterator() (*Pctmpu1interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu1interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu1interrMetricsIterator{iter: iter}, nil
}

type Pctmpu1intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu1intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu1intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu1intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu1intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pctmpu1intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pctmpu1intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pctmpu1intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pctmpu1intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pctmpu1intinfoMetricsIterator is the iterator object
type Pctmpu1intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu1intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu1intinfoMetricsIterator) Next() *Pctmpu1intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu1intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu1intinfoMetricsIterator) Find(key uint64) (*Pctmpu1intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu1intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu1intinfoMetricsIterator) Create(key uint64) (*Pctmpu1intinfoMetrics, error) {
	tmtr := &Pctmpu1intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu1intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu1intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu1intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu1intinfoMetricsIterator returns an iterator
func NewPctmpu1intinfoMetricsIterator() (*Pctmpu1intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu1intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu1intinfoMetricsIterator{iter: iter}, nil
}

type Pctmpu2interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu2interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu2interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu2interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu2interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pctmpu2interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pctmpu2interrMetricsIterator is the iterator object
type Pctmpu2interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu2interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu2interrMetricsIterator) Next() *Pctmpu2interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu2interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu2interrMetricsIterator) Find(key uint64) (*Pctmpu2interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu2interrMetricsIterator) Create(key uint64) (*Pctmpu2interrMetrics, error) {
	tmtr := &Pctmpu2interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu2interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu2interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu2interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu2interrMetricsIterator returns an iterator
func NewPctmpu2interrMetricsIterator() (*Pctmpu2interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu2interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu2interrMetricsIterator{iter: iter}, nil
}

type Pctmpu2intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu2intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu2intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu2intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu2intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pctmpu2intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pctmpu2intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pctmpu2intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pctmpu2intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pctmpu2intinfoMetricsIterator is the iterator object
type Pctmpu2intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu2intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu2intinfoMetricsIterator) Next() *Pctmpu2intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu2intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu2intinfoMetricsIterator) Find(key uint64) (*Pctmpu2intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu2intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu2intinfoMetricsIterator) Create(key uint64) (*Pctmpu2intinfoMetrics, error) {
	tmtr := &Pctmpu2intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu2intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu2intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu2intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu2intinfoMetricsIterator returns an iterator
func NewPctmpu2intinfoMetricsIterator() (*Pctmpu2intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu2intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu2intinfoMetricsIterator{iter: iter}, nil
}

type Pctmpu3interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu3interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu3interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu3interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu3interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pctmpu3interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pctmpu3interrMetricsIterator is the iterator object
type Pctmpu3interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu3interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu3interrMetricsIterator) Next() *Pctmpu3interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu3interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu3interrMetricsIterator) Find(key uint64) (*Pctmpu3interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu3interrMetricsIterator) Create(key uint64) (*Pctmpu3interrMetrics, error) {
	tmtr := &Pctmpu3interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu3interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu3interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu3interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu3interrMetricsIterator returns an iterator
func NewPctmpu3interrMetricsIterator() (*Pctmpu3interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu3interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu3interrMetricsIterator{iter: iter}, nil
}

type Pctmpu3intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu3intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu3intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu3intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu3intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pctmpu3intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pctmpu3intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pctmpu3intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pctmpu3intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pctmpu3intinfoMetricsIterator is the iterator object
type Pctmpu3intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu3intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu3intinfoMetricsIterator) Next() *Pctmpu3intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu3intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu3intinfoMetricsIterator) Find(key uint64) (*Pctmpu3intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu3intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu3intinfoMetricsIterator) Create(key uint64) (*Pctmpu3intinfoMetrics, error) {
	tmtr := &Pctmpu3intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu3intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu3intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu3intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu3intinfoMetricsIterator returns an iterator
func NewPctmpu3intinfoMetricsIterator() (*Pctmpu3intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu3intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu3intinfoMetricsIterator{iter: iter}, nil
}

type Pctmpu4interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu4interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu4interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu4interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu4interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pctmpu4interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pctmpu4interrMetricsIterator is the iterator object
type Pctmpu4interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu4interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu4interrMetricsIterator) Next() *Pctmpu4interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu4interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu4interrMetricsIterator) Find(key uint64) (*Pctmpu4interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu4interrMetricsIterator) Create(key uint64) (*Pctmpu4interrMetrics, error) {
	tmtr := &Pctmpu4interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu4interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu4interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu4interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu4interrMetricsIterator returns an iterator
func NewPctmpu4interrMetricsIterator() (*Pctmpu4interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu4interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu4interrMetricsIterator{iter: iter}, nil
}

type Pctmpu4intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu4intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu4intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu4intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu4intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pctmpu4intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pctmpu4intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pctmpu4intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pctmpu4intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pctmpu4intinfoMetricsIterator is the iterator object
type Pctmpu4intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu4intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu4intinfoMetricsIterator) Next() *Pctmpu4intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu4intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu4intinfoMetricsIterator) Find(key uint64) (*Pctmpu4intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu4intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu4intinfoMetricsIterator) Create(key uint64) (*Pctmpu4intinfoMetrics, error) {
	tmtr := &Pctmpu4intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu4intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu4intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu4intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu4intinfoMetricsIterator returns an iterator
func NewPctmpu4intinfoMetricsIterator() (*Pctmpu4intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu4intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu4intinfoMetricsIterator{iter: iter}, nil
}

type Pctmpu5interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu5interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu5interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu5interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu5interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pctmpu5interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pctmpu5interrMetricsIterator is the iterator object
type Pctmpu5interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu5interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu5interrMetricsIterator) Next() *Pctmpu5interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu5interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu5interrMetricsIterator) Find(key uint64) (*Pctmpu5interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu5interrMetricsIterator) Create(key uint64) (*Pctmpu5interrMetrics, error) {
	tmtr := &Pctmpu5interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu5interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu5interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu5interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu5interrMetricsIterator returns an iterator
func NewPctmpu5interrMetricsIterator() (*Pctmpu5interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu5interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu5interrMetricsIterator{iter: iter}, nil
}

type Pctmpu5intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu5intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu5intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu5intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu5intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pctmpu5intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pctmpu5intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pctmpu5intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pctmpu5intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pctmpu5intinfoMetricsIterator is the iterator object
type Pctmpu5intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu5intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu5intinfoMetricsIterator) Next() *Pctmpu5intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu5intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu5intinfoMetricsIterator) Find(key uint64) (*Pctmpu5intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu5intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu5intinfoMetricsIterator) Create(key uint64) (*Pctmpu5intinfoMetrics, error) {
	tmtr := &Pctmpu5intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu5intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu5intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu5intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu5intinfoMetricsIterator returns an iterator
func NewPctmpu5intinfoMetricsIterator() (*Pctmpu5intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu5intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu5intinfoMetricsIterator{iter: iter}, nil
}

type Pctmpu6interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu6interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu6interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu6interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu6interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pctmpu6interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pctmpu6interrMetricsIterator is the iterator object
type Pctmpu6interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu6interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu6interrMetricsIterator) Next() *Pctmpu6interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu6interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu6interrMetricsIterator) Find(key uint64) (*Pctmpu6interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu6interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu6interrMetricsIterator) Create(key uint64) (*Pctmpu6interrMetrics, error) {
	tmtr := &Pctmpu6interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu6interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu6interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu6interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu6interrMetricsIterator returns an iterator
func NewPctmpu6interrMetricsIterator() (*Pctmpu6interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu6interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu6interrMetricsIterator{iter: iter}, nil
}

type Pctmpu6intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu6intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu6intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu6intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu6intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pctmpu6intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pctmpu6intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pctmpu6intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pctmpu6intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pctmpu6intinfoMetricsIterator is the iterator object
type Pctmpu6intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu6intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu6intinfoMetricsIterator) Next() *Pctmpu6intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu6intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu6intinfoMetricsIterator) Find(key uint64) (*Pctmpu6intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu6intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu6intinfoMetricsIterator) Create(key uint64) (*Pctmpu6intinfoMetrics, error) {
	tmtr := &Pctmpu6intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu6intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu6intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu6intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu6intinfoMetricsIterator returns an iterator
func NewPctmpu6intinfoMetricsIterator() (*Pctmpu6intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu6intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu6intinfoMetricsIterator{iter: iter}, nil
}

type Pctmpu7interrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SdpMemUncorrectable metrics.Counter

	SdpMemCorrectable metrics.Counter

	IllegalOp_0 metrics.Counter

	IllegalOp_1 metrics.Counter

	IllegalOp_2 metrics.Counter

	IllegalOp_3 metrics.Counter

	MaxInst_0 metrics.Counter

	MaxInst_1 metrics.Counter

	MaxInst_2 metrics.Counter

	MaxInst_3 metrics.Counter

	Phvwr_0 metrics.Counter

	Phvwr_1 metrics.Counter

	Phvwr_2 metrics.Counter

	Phvwr_3 metrics.Counter

	WriteErr_0 metrics.Counter

	WriteErr_1 metrics.Counter

	WriteErr_2 metrics.Counter

	WriteErr_3 metrics.Counter

	CacheAxi_0 metrics.Counter

	CacheAxi_1 metrics.Counter

	CacheAxi_2 metrics.Counter

	CacheAxi_3 metrics.Counter

	CacheParity_0 metrics.Counter

	CacheParity_1 metrics.Counter

	CacheParity_2 metrics.Counter

	CacheParity_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu7interrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu7interrMetrics) Size() int {
	sz := 0

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu7interrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu7interrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetSdpMemUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetSdpMemCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetIllegalOp_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetIllegalOp_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetIllegalOp_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetIllegalOp_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetMaxInst_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetMaxInst_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetMaxInst_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetMaxInst_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetPhvwr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetPhvwr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetPhvwr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetPhvwr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetWriteErr_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetWriteErr_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetWriteErr_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetWriteErr_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetCacheAxi_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetCacheAxi_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetCacheAxi_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetCacheAxi_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetCacheParity_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetCacheParity_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetCacheParity_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *Pctmpu7interrMetrics) SetCacheParity_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// Pctmpu7interrMetricsIterator is the iterator object
type Pctmpu7interrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu7interrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu7interrMetricsIterator) Next() *Pctmpu7interrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu7interrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu7interrMetricsIterator) Find(key uint64) (*Pctmpu7interrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu7interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu7interrMetricsIterator) Create(key uint64) (*Pctmpu7interrMetrics, error) {
	tmtr := &Pctmpu7interrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu7interrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu7interrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu7interrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu7interrMetricsIterator returns an iterator
func NewPctmpu7interrMetricsIterator() (*Pctmpu7interrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu7interrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu7interrMetricsIterator{iter: iter}, nil
}

type Pctmpu7intinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MpuStop_0 metrics.Counter

	MpuStop_1 metrics.Counter

	MpuStop_2 metrics.Counter

	MpuStop_3 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pctmpu7intinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pctmpu7intinfoMetrics) Size() int {
	sz := 0

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pctmpu7intinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pctmpu7intinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *Pctmpu7intinfoMetrics) SetMpuStop_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *Pctmpu7intinfoMetrics) SetMpuStop_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *Pctmpu7intinfoMetrics) SetMpuStop_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *Pctmpu7intinfoMetrics) SetMpuStop_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// Pctmpu7intinfoMetricsIterator is the iterator object
type Pctmpu7intinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pctmpu7intinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pctmpu7intinfoMetricsIterator) Next() *Pctmpu7intinfoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pctmpu7intinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pctmpu7intinfoMetricsIterator) Find(key uint64) (*Pctmpu7intinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pctmpu7intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pctmpu7intinfoMetricsIterator) Create(key uint64) (*Pctmpu7intinfoMetrics, error) {
	tmtr := &Pctmpu7intinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pctmpu7intinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pctmpu7intinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pctmpu7intinfoMetricsIterator) Free() {
	it.iter.Free()
}

// NewPctmpu7intinfoMetricsIterator returns an iterator
func NewPctmpu7intinfoMetricsIterator() (*Pctmpu7intinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pctmpu7intinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pctmpu7intinfoMetricsIterator{iter: iter}, nil
}

type Pbpbcinteccfc0Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcinteccfc0Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcinteccfc0Metrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcinteccfc0Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcinteccfc0Metrics) getOffset(fldName string) int {
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
func (mtr *Pbpbcinteccfc0Metrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *Pbpbcinteccfc0Metrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// Pbpbcinteccfc0MetricsIterator is the iterator object
type Pbpbcinteccfc0MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcinteccfc0MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcinteccfc0MetricsIterator) Next() *Pbpbcinteccfc0Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcinteccfc0Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcinteccfc0MetricsIterator) Find(key uint64) (*Pbpbcinteccfc0Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcinteccfc0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcinteccfc0MetricsIterator) Create(key uint64) (*Pbpbcinteccfc0Metrics, error) {
	tmtr := &Pbpbcinteccfc0Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcinteccfc0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcinteccfc0MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcinteccfc0MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccfc0MetricsIterator returns an iterator
func NewPbpbcinteccfc0MetricsIterator() (*Pbpbcinteccfc0MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcinteccfc0Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcinteccfc0MetricsIterator{iter: iter}, nil
}

type Pbpbcinteccll0Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcinteccll0Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcinteccll0Metrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcinteccll0Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcinteccll0Metrics) getOffset(fldName string) int {
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
func (mtr *Pbpbcinteccll0Metrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *Pbpbcinteccll0Metrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// Pbpbcinteccll0MetricsIterator is the iterator object
type Pbpbcinteccll0MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcinteccll0MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcinteccll0MetricsIterator) Next() *Pbpbcinteccll0Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcinteccll0Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcinteccll0MetricsIterator) Find(key uint64) (*Pbpbcinteccll0Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcinteccll0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcinteccll0MetricsIterator) Create(key uint64) (*Pbpbcinteccll0Metrics, error) {
	tmtr := &Pbpbcinteccll0Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcinteccll0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcinteccll0MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcinteccll0MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccll0MetricsIterator returns an iterator
func NewPbpbcinteccll0MetricsIterator() (*Pbpbcinteccll0MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcinteccll0Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcinteccll0MetricsIterator{iter: iter}, nil
}

type Pbpbcinteccdesc0Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcinteccdesc0Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcinteccdesc0Metrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcinteccdesc0Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcinteccdesc0Metrics) getOffset(fldName string) int {
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
func (mtr *Pbpbcinteccdesc0Metrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *Pbpbcinteccdesc0Metrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// Pbpbcinteccdesc0MetricsIterator is the iterator object
type Pbpbcinteccdesc0MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcinteccdesc0MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcinteccdesc0MetricsIterator) Next() *Pbpbcinteccdesc0Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcinteccdesc0Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcinteccdesc0MetricsIterator) Find(key uint64) (*Pbpbcinteccdesc0Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcinteccdesc0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcinteccdesc0MetricsIterator) Create(key uint64) (*Pbpbcinteccdesc0Metrics, error) {
	tmtr := &Pbpbcinteccdesc0Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcinteccdesc0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcinteccdesc0MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcinteccdesc0MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccdesc0MetricsIterator returns an iterator
func NewPbpbcinteccdesc0MetricsIterator() (*Pbpbcinteccdesc0MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcinteccdesc0Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcinteccdesc0MetricsIterator{iter: iter}, nil
}

type Pbpbcinteccfc1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcinteccfc1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcinteccfc1Metrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcinteccfc1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcinteccfc1Metrics) getOffset(fldName string) int {
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
func (mtr *Pbpbcinteccfc1Metrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *Pbpbcinteccfc1Metrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// Pbpbcinteccfc1MetricsIterator is the iterator object
type Pbpbcinteccfc1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcinteccfc1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcinteccfc1MetricsIterator) Next() *Pbpbcinteccfc1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcinteccfc1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcinteccfc1MetricsIterator) Find(key uint64) (*Pbpbcinteccfc1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcinteccfc1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcinteccfc1MetricsIterator) Create(key uint64) (*Pbpbcinteccfc1Metrics, error) {
	tmtr := &Pbpbcinteccfc1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcinteccfc1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcinteccfc1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcinteccfc1MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccfc1MetricsIterator returns an iterator
func NewPbpbcinteccfc1MetricsIterator() (*Pbpbcinteccfc1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcinteccfc1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcinteccfc1MetricsIterator{iter: iter}, nil
}

type Pbpbcinteccll1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcinteccll1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcinteccll1Metrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcinteccll1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcinteccll1Metrics) getOffset(fldName string) int {
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
func (mtr *Pbpbcinteccll1Metrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *Pbpbcinteccll1Metrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// Pbpbcinteccll1MetricsIterator is the iterator object
type Pbpbcinteccll1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcinteccll1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcinteccll1MetricsIterator) Next() *Pbpbcinteccll1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcinteccll1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcinteccll1MetricsIterator) Find(key uint64) (*Pbpbcinteccll1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcinteccll1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcinteccll1MetricsIterator) Create(key uint64) (*Pbpbcinteccll1Metrics, error) {
	tmtr := &Pbpbcinteccll1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcinteccll1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcinteccll1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcinteccll1MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccll1MetricsIterator returns an iterator
func NewPbpbcinteccll1MetricsIterator() (*Pbpbcinteccll1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcinteccll1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcinteccll1MetricsIterator{iter: iter}, nil
}

type Pbpbcinteccdesc1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcinteccdesc1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcinteccdesc1Metrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcinteccdesc1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcinteccdesc1Metrics) getOffset(fldName string) int {
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
func (mtr *Pbpbcinteccdesc1Metrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *Pbpbcinteccdesc1Metrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// Pbpbcinteccdesc1MetricsIterator is the iterator object
type Pbpbcinteccdesc1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcinteccdesc1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcinteccdesc1MetricsIterator) Next() *Pbpbcinteccdesc1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcinteccdesc1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcinteccdesc1MetricsIterator) Find(key uint64) (*Pbpbcinteccdesc1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcinteccdesc1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcinteccdesc1MetricsIterator) Create(key uint64) (*Pbpbcinteccdesc1Metrics, error) {
	tmtr := &Pbpbcinteccdesc1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcinteccdesc1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcinteccdesc1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcinteccdesc1MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccdesc1MetricsIterator returns an iterator
func NewPbpbcinteccdesc1MetricsIterator() (*Pbpbcinteccdesc1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcinteccdesc1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcinteccdesc1MetricsIterator{iter: iter}, nil
}

type Pbpbcintwrite0Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CreditGrowthError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcintwrite0Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcintwrite0Metrics) Size() int {
	sz := 0

	sz += mtr.CreditGrowthError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcintwrite0Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CreditGrowthError = mtr.metrics.GetCounter(offset)
	offset += mtr.CreditGrowthError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcintwrite0Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CreditGrowthError" {
		return offset
	}
	offset += mtr.CreditGrowthError.Size()

	return offset
}

// SetCreditGrowthError sets cunter in shared memory
func (mtr *Pbpbcintwrite0Metrics) SetCreditGrowthError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CreditGrowthError"))
	return nil
}

// Pbpbcintwrite0MetricsIterator is the iterator object
type Pbpbcintwrite0MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcintwrite0MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcintwrite0MetricsIterator) Next() *Pbpbcintwrite0Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcintwrite0Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcintwrite0MetricsIterator) Find(key uint64) (*Pbpbcintwrite0Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcintwrite0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcintwrite0MetricsIterator) Create(key uint64) (*Pbpbcintwrite0Metrics, error) {
	tmtr := &Pbpbcintwrite0Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcintwrite0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcintwrite0MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcintwrite0MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintwrite0MetricsIterator returns an iterator
func NewPbpbcintwrite0MetricsIterator() (*Pbpbcintwrite0MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcintwrite0Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcintwrite0MetricsIterator{iter: iter}, nil
}

type Pbpbcintwrite1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CreditGrowthError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcintwrite1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcintwrite1Metrics) Size() int {
	sz := 0

	sz += mtr.CreditGrowthError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcintwrite1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CreditGrowthError = mtr.metrics.GetCounter(offset)
	offset += mtr.CreditGrowthError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcintwrite1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CreditGrowthError" {
		return offset
	}
	offset += mtr.CreditGrowthError.Size()

	return offset
}

// SetCreditGrowthError sets cunter in shared memory
func (mtr *Pbpbcintwrite1Metrics) SetCreditGrowthError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CreditGrowthError"))
	return nil
}

// Pbpbcintwrite1MetricsIterator is the iterator object
type Pbpbcintwrite1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcintwrite1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcintwrite1MetricsIterator) Next() *Pbpbcintwrite1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcintwrite1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcintwrite1MetricsIterator) Find(key uint64) (*Pbpbcintwrite1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcintwrite1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcintwrite1MetricsIterator) Create(key uint64) (*Pbpbcintwrite1Metrics, error) {
	tmtr := &Pbpbcintwrite1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcintwrite1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcintwrite1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcintwrite1MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintwrite1MetricsIterator returns an iterator
func NewPbpbcintwrite1MetricsIterator() (*Pbpbcintwrite1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcintwrite1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcintwrite1MetricsIterator{iter: iter}, nil
}

type Pbpbcintwrite2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CreditGrowthError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcintwrite2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcintwrite2Metrics) Size() int {
	sz := 0

	sz += mtr.CreditGrowthError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcintwrite2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CreditGrowthError = mtr.metrics.GetCounter(offset)
	offset += mtr.CreditGrowthError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcintwrite2Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CreditGrowthError" {
		return offset
	}
	offset += mtr.CreditGrowthError.Size()

	return offset
}

// SetCreditGrowthError sets cunter in shared memory
func (mtr *Pbpbcintwrite2Metrics) SetCreditGrowthError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CreditGrowthError"))
	return nil
}

// Pbpbcintwrite2MetricsIterator is the iterator object
type Pbpbcintwrite2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcintwrite2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcintwrite2MetricsIterator) Next() *Pbpbcintwrite2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcintwrite2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcintwrite2MetricsIterator) Find(key uint64) (*Pbpbcintwrite2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcintwrite2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcintwrite2MetricsIterator) Create(key uint64) (*Pbpbcintwrite2Metrics, error) {
	tmtr := &Pbpbcintwrite2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcintwrite2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcintwrite2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcintwrite2MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintwrite2MetricsIterator returns an iterator
func NewPbpbcintwrite2MetricsIterator() (*Pbpbcintwrite2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcintwrite2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcintwrite2MetricsIterator{iter: iter}, nil
}

type Pbpbcintwrite3Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CreditGrowthError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcintwrite3Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcintwrite3Metrics) Size() int {
	sz := 0

	sz += mtr.CreditGrowthError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcintwrite3Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CreditGrowthError = mtr.metrics.GetCounter(offset)
	offset += mtr.CreditGrowthError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcintwrite3Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CreditGrowthError" {
		return offset
	}
	offset += mtr.CreditGrowthError.Size()

	return offset
}

// SetCreditGrowthError sets cunter in shared memory
func (mtr *Pbpbcintwrite3Metrics) SetCreditGrowthError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CreditGrowthError"))
	return nil
}

// Pbpbcintwrite3MetricsIterator is the iterator object
type Pbpbcintwrite3MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcintwrite3MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcintwrite3MetricsIterator) Next() *Pbpbcintwrite3Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcintwrite3Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcintwrite3MetricsIterator) Find(key uint64) (*Pbpbcintwrite3Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcintwrite3Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcintwrite3MetricsIterator) Create(key uint64) (*Pbpbcintwrite3Metrics, error) {
	tmtr := &Pbpbcintwrite3Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcintwrite3Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcintwrite3MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcintwrite3MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintwrite3MetricsIterator returns an iterator
func NewPbpbcintwrite3MetricsIterator() (*Pbpbcintwrite3MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcintwrite3Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcintwrite3MetricsIterator{iter: iter}, nil
}

type Pbpbcintwrite4Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CreditGrowthError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcintwrite4Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcintwrite4Metrics) Size() int {
	sz := 0

	sz += mtr.CreditGrowthError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcintwrite4Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CreditGrowthError = mtr.metrics.GetCounter(offset)
	offset += mtr.CreditGrowthError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcintwrite4Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CreditGrowthError" {
		return offset
	}
	offset += mtr.CreditGrowthError.Size()

	return offset
}

// SetCreditGrowthError sets cunter in shared memory
func (mtr *Pbpbcintwrite4Metrics) SetCreditGrowthError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CreditGrowthError"))
	return nil
}

// Pbpbcintwrite4MetricsIterator is the iterator object
type Pbpbcintwrite4MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcintwrite4MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcintwrite4MetricsIterator) Next() *Pbpbcintwrite4Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcintwrite4Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcintwrite4MetricsIterator) Find(key uint64) (*Pbpbcintwrite4Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcintwrite4Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcintwrite4MetricsIterator) Create(key uint64) (*Pbpbcintwrite4Metrics, error) {
	tmtr := &Pbpbcintwrite4Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcintwrite4Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcintwrite4MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcintwrite4MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintwrite4MetricsIterator returns an iterator
func NewPbpbcintwrite4MetricsIterator() (*Pbpbcintwrite4MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcintwrite4Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcintwrite4MetricsIterator{iter: iter}, nil
}

type Pbpbcintwrite5Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CreditGrowthError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcintwrite5Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcintwrite5Metrics) Size() int {
	sz := 0

	sz += mtr.CreditGrowthError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcintwrite5Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CreditGrowthError = mtr.metrics.GetCounter(offset)
	offset += mtr.CreditGrowthError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcintwrite5Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CreditGrowthError" {
		return offset
	}
	offset += mtr.CreditGrowthError.Size()

	return offset
}

// SetCreditGrowthError sets cunter in shared memory
func (mtr *Pbpbcintwrite5Metrics) SetCreditGrowthError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CreditGrowthError"))
	return nil
}

// Pbpbcintwrite5MetricsIterator is the iterator object
type Pbpbcintwrite5MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcintwrite5MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcintwrite5MetricsIterator) Next() *Pbpbcintwrite5Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcintwrite5Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcintwrite5MetricsIterator) Find(key uint64) (*Pbpbcintwrite5Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcintwrite5Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcintwrite5MetricsIterator) Create(key uint64) (*Pbpbcintwrite5Metrics, error) {
	tmtr := &Pbpbcintwrite5Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcintwrite5Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcintwrite5MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcintwrite5MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintwrite5MetricsIterator returns an iterator
func NewPbpbcintwrite5MetricsIterator() (*Pbpbcintwrite5MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcintwrite5Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcintwrite5MetricsIterator{iter: iter}, nil
}

type Pbpbcintwrite6Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CreditGrowthError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcintwrite6Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcintwrite6Metrics) Size() int {
	sz := 0

	sz += mtr.CreditGrowthError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcintwrite6Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CreditGrowthError = mtr.metrics.GetCounter(offset)
	offset += mtr.CreditGrowthError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcintwrite6Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CreditGrowthError" {
		return offset
	}
	offset += mtr.CreditGrowthError.Size()

	return offset
}

// SetCreditGrowthError sets cunter in shared memory
func (mtr *Pbpbcintwrite6Metrics) SetCreditGrowthError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CreditGrowthError"))
	return nil
}

// Pbpbcintwrite6MetricsIterator is the iterator object
type Pbpbcintwrite6MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcintwrite6MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcintwrite6MetricsIterator) Next() *Pbpbcintwrite6Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcintwrite6Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcintwrite6MetricsIterator) Find(key uint64) (*Pbpbcintwrite6Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcintwrite6Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcintwrite6MetricsIterator) Create(key uint64) (*Pbpbcintwrite6Metrics, error) {
	tmtr := &Pbpbcintwrite6Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcintwrite6Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcintwrite6MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcintwrite6MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintwrite6MetricsIterator returns an iterator
func NewPbpbcintwrite6MetricsIterator() (*Pbpbcintwrite6MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcintwrite6Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcintwrite6MetricsIterator{iter: iter}, nil
}

type Pbpbcintwrite7Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CreditGrowthError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcintwrite7Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcintwrite7Metrics) Size() int {
	sz := 0

	sz += mtr.CreditGrowthError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcintwrite7Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CreditGrowthError = mtr.metrics.GetCounter(offset)
	offset += mtr.CreditGrowthError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcintwrite7Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CreditGrowthError" {
		return offset
	}
	offset += mtr.CreditGrowthError.Size()

	return offset
}

// SetCreditGrowthError sets cunter in shared memory
func (mtr *Pbpbcintwrite7Metrics) SetCreditGrowthError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CreditGrowthError"))
	return nil
}

// Pbpbcintwrite7MetricsIterator is the iterator object
type Pbpbcintwrite7MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcintwrite7MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcintwrite7MetricsIterator) Next() *Pbpbcintwrite7Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcintwrite7Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcintwrite7MetricsIterator) Find(key uint64) (*Pbpbcintwrite7Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcintwrite7Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcintwrite7MetricsIterator) Create(key uint64) (*Pbpbcintwrite7Metrics, error) {
	tmtr := &Pbpbcintwrite7Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcintwrite7Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcintwrite7MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcintwrite7MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintwrite7MetricsIterator returns an iterator
func NewPbpbcintwrite7MetricsIterator() (*Pbpbcintwrite7MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcintwrite7Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcintwrite7MetricsIterator{iter: iter}, nil
}

type Pbpbcintwrite8Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CreditGrowthError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcintwrite8Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcintwrite8Metrics) Size() int {
	sz := 0

	sz += mtr.CreditGrowthError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcintwrite8Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CreditGrowthError = mtr.metrics.GetCounter(offset)
	offset += mtr.CreditGrowthError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcintwrite8Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CreditGrowthError" {
		return offset
	}
	offset += mtr.CreditGrowthError.Size()

	return offset
}

// SetCreditGrowthError sets cunter in shared memory
func (mtr *Pbpbcintwrite8Metrics) SetCreditGrowthError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CreditGrowthError"))
	return nil
}

// Pbpbcintwrite8MetricsIterator is the iterator object
type Pbpbcintwrite8MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcintwrite8MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcintwrite8MetricsIterator) Next() *Pbpbcintwrite8Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcintwrite8Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcintwrite8MetricsIterator) Find(key uint64) (*Pbpbcintwrite8Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcintwrite8Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcintwrite8MetricsIterator) Create(key uint64) (*Pbpbcintwrite8Metrics, error) {
	tmtr := &Pbpbcintwrite8Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcintwrite8Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcintwrite8MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcintwrite8MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintwrite8MetricsIterator returns an iterator
func NewPbpbcintwrite8MetricsIterator() (*Pbpbcintwrite8MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcintwrite8Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcintwrite8MetricsIterator{iter: iter}, nil
}

type Pbpbcintwrite9Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CreditGrowthError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcintwrite9Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcintwrite9Metrics) Size() int {
	sz := 0

	sz += mtr.CreditGrowthError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcintwrite9Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CreditGrowthError = mtr.metrics.GetCounter(offset)
	offset += mtr.CreditGrowthError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcintwrite9Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CreditGrowthError" {
		return offset
	}
	offset += mtr.CreditGrowthError.Size()

	return offset
}

// SetCreditGrowthError sets cunter in shared memory
func (mtr *Pbpbcintwrite9Metrics) SetCreditGrowthError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CreditGrowthError"))
	return nil
}

// Pbpbcintwrite9MetricsIterator is the iterator object
type Pbpbcintwrite9MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcintwrite9MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcintwrite9MetricsIterator) Next() *Pbpbcintwrite9Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcintwrite9Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcintwrite9MetricsIterator) Find(key uint64) (*Pbpbcintwrite9Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcintwrite9Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcintwrite9MetricsIterator) Create(key uint64) (*Pbpbcintwrite9Metrics, error) {
	tmtr := &Pbpbcintwrite9Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcintwrite9Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcintwrite9MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcintwrite9MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintwrite9MetricsIterator returns an iterator
func NewPbpbcintwrite9MetricsIterator() (*Pbpbcintwrite9MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcintwrite9Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcintwrite9MetricsIterator{iter: iter}, nil
}

type Pbpbcintwrite10Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CreditGrowthError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcintwrite10Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcintwrite10Metrics) Size() int {
	sz := 0

	sz += mtr.CreditGrowthError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcintwrite10Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CreditGrowthError = mtr.metrics.GetCounter(offset)
	offset += mtr.CreditGrowthError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcintwrite10Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CreditGrowthError" {
		return offset
	}
	offset += mtr.CreditGrowthError.Size()

	return offset
}

// SetCreditGrowthError sets cunter in shared memory
func (mtr *Pbpbcintwrite10Metrics) SetCreditGrowthError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CreditGrowthError"))
	return nil
}

// Pbpbcintwrite10MetricsIterator is the iterator object
type Pbpbcintwrite10MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcintwrite10MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcintwrite10MetricsIterator) Next() *Pbpbcintwrite10Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcintwrite10Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcintwrite10MetricsIterator) Find(key uint64) (*Pbpbcintwrite10Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcintwrite10Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcintwrite10MetricsIterator) Create(key uint64) (*Pbpbcintwrite10Metrics, error) {
	tmtr := &Pbpbcintwrite10Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcintwrite10Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcintwrite10MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcintwrite10MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintwrite10MetricsIterator returns an iterator
func NewPbpbcintwrite10MetricsIterator() (*Pbpbcintwrite10MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcintwrite10Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcintwrite10MetricsIterator{iter: iter}, nil
}

type Pbpbcintwrite11Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CreditGrowthError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Pbpbcintwrite11Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Pbpbcintwrite11Metrics) Size() int {
	sz := 0

	sz += mtr.CreditGrowthError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Pbpbcintwrite11Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CreditGrowthError = mtr.metrics.GetCounter(offset)
	offset += mtr.CreditGrowthError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Pbpbcintwrite11Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CreditGrowthError" {
		return offset
	}
	offset += mtr.CreditGrowthError.Size()

	return offset
}

// SetCreditGrowthError sets cunter in shared memory
func (mtr *Pbpbcintwrite11Metrics) SetCreditGrowthError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CreditGrowthError"))
	return nil
}

// Pbpbcintwrite11MetricsIterator is the iterator object
type Pbpbcintwrite11MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Pbpbcintwrite11MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Pbpbcintwrite11MetricsIterator) Next() *Pbpbcintwrite11Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Pbpbcintwrite11Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Pbpbcintwrite11MetricsIterator) Find(key uint64) (*Pbpbcintwrite11Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Pbpbcintwrite11Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Pbpbcintwrite11MetricsIterator) Create(key uint64) (*Pbpbcintwrite11Metrics, error) {
	tmtr := &Pbpbcintwrite11Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Pbpbcintwrite11Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Pbpbcintwrite11MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Pbpbcintwrite11MetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintwrite11MetricsIterator returns an iterator
func NewPbpbcintwrite11MetricsIterator() (*Pbpbcintwrite11MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Pbpbcintwrite11Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Pbpbcintwrite11MetricsIterator{iter: iter}, nil
}

type PbpbcinteccrcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcinteccrcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcinteccrcMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcinteccrcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcinteccrcMetrics) getOffset(fldName string) int {
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
func (mtr *PbpbcinteccrcMetrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *PbpbcinteccrcMetrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// PbpbcinteccrcMetricsIterator is the iterator object
type PbpbcinteccrcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcinteccrcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcinteccrcMetricsIterator) Next() *PbpbcinteccrcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbcinteccrcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcinteccrcMetricsIterator) Find(key uint64) (*PbpbcinteccrcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcinteccrcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcinteccrcMetricsIterator) Create(key uint64) (*PbpbcinteccrcMetrics, error) {
	tmtr := &PbpbcinteccrcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcinteccrcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcinteccrcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbcinteccrcMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccrcMetricsIterator returns an iterator
func NewPbpbcinteccrcMetricsIterator() (*PbpbcinteccrcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcinteccrcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcinteccrcMetricsIterator{iter: iter}, nil
}

type PbpbcinteccpackMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcinteccpackMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcinteccpackMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcinteccpackMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcinteccpackMetrics) getOffset(fldName string) int {
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
func (mtr *PbpbcinteccpackMetrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *PbpbcinteccpackMetrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// PbpbcinteccpackMetricsIterator is the iterator object
type PbpbcinteccpackMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcinteccpackMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcinteccpackMetricsIterator) Next() *PbpbcinteccpackMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbcinteccpackMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcinteccpackMetricsIterator) Find(key uint64) (*PbpbcinteccpackMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcinteccpackMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcinteccpackMetricsIterator) Create(key uint64) (*PbpbcinteccpackMetrics, error) {
	tmtr := &PbpbcinteccpackMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcinteccpackMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcinteccpackMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbcinteccpackMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccpackMetricsIterator returns an iterator
func NewPbpbcinteccpackMetricsIterator() (*PbpbcinteccpackMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcinteccpackMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcinteccpackMetricsIterator{iter: iter}, nil
}

type PbpbcinteccrwrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcinteccrwrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcinteccrwrMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcinteccrwrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcinteccrwrMetrics) getOffset(fldName string) int {
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
func (mtr *PbpbcinteccrwrMetrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *PbpbcinteccrwrMetrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// PbpbcinteccrwrMetricsIterator is the iterator object
type PbpbcinteccrwrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcinteccrwrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcinteccrwrMetricsIterator) Next() *PbpbcinteccrwrMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbcinteccrwrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcinteccrwrMetricsIterator) Find(key uint64) (*PbpbcinteccrwrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcinteccrwrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcinteccrwrMetricsIterator) Create(key uint64) (*PbpbcinteccrwrMetrics, error) {
	tmtr := &PbpbcinteccrwrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcinteccrwrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcinteccrwrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbcinteccrwrMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccrwrMetricsIterator returns an iterator
func NewPbpbcinteccrwrMetricsIterator() (*PbpbcinteccrwrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcinteccrwrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcinteccrwrMetricsIterator{iter: iter}, nil
}

type PbpbcinteccschedMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcinteccschedMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcinteccschedMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcinteccschedMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcinteccschedMetrics) getOffset(fldName string) int {
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
func (mtr *PbpbcinteccschedMetrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *PbpbcinteccschedMetrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// PbpbcinteccschedMetricsIterator is the iterator object
type PbpbcinteccschedMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcinteccschedMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcinteccschedMetricsIterator) Next() *PbpbcinteccschedMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbcinteccschedMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcinteccschedMetricsIterator) Find(key uint64) (*PbpbcinteccschedMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcinteccschedMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcinteccschedMetricsIterator) Create(key uint64) (*PbpbcinteccschedMetrics, error) {
	tmtr := &PbpbcinteccschedMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcinteccschedMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcinteccschedMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbcinteccschedMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccschedMetricsIterator returns an iterator
func NewPbpbcinteccschedMetricsIterator() (*PbpbcinteccschedMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcinteccschedMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcinteccschedMetricsIterator{iter: iter}, nil
}

type PbpbcinteccsidebandMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PckUncorrectable metrics.Counter

	PckCorrectable metrics.Counter

	RwrUncorrectable metrics.Counter

	RwrCorrectable metrics.Counter

	RxdUncorrectable metrics.Counter

	RxdCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcinteccsidebandMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcinteccsidebandMetrics) Size() int {
	sz := 0

	sz += mtr.PckUncorrectable.Size()

	sz += mtr.PckCorrectable.Size()

	sz += mtr.RwrUncorrectable.Size()

	sz += mtr.RwrCorrectable.Size()

	sz += mtr.RxdUncorrectable.Size()

	sz += mtr.RxdCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcinteccsidebandMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PckUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PckUncorrectable.Size()

	mtr.PckCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.PckCorrectable.Size()

	mtr.RwrUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RwrUncorrectable.Size()

	mtr.RwrCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RwrCorrectable.Size()

	mtr.RxdUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RxdUncorrectable.Size()

	mtr.RxdCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RxdCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcinteccsidebandMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PckUncorrectable" {
		return offset
	}
	offset += mtr.PckUncorrectable.Size()

	if fldName == "PckCorrectable" {
		return offset
	}
	offset += mtr.PckCorrectable.Size()

	if fldName == "RwrUncorrectable" {
		return offset
	}
	offset += mtr.RwrUncorrectable.Size()

	if fldName == "RwrCorrectable" {
		return offset
	}
	offset += mtr.RwrCorrectable.Size()

	if fldName == "RxdUncorrectable" {
		return offset
	}
	offset += mtr.RxdUncorrectable.Size()

	if fldName == "RxdCorrectable" {
		return offset
	}
	offset += mtr.RxdCorrectable.Size()

	return offset
}

// SetPckUncorrectable sets cunter in shared memory
func (mtr *PbpbcinteccsidebandMetrics) SetPckUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PckUncorrectable"))
	return nil
}

// SetPckCorrectable sets cunter in shared memory
func (mtr *PbpbcinteccsidebandMetrics) SetPckCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PckCorrectable"))
	return nil
}

// SetRwrUncorrectable sets cunter in shared memory
func (mtr *PbpbcinteccsidebandMetrics) SetRwrUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RwrUncorrectable"))
	return nil
}

// SetRwrCorrectable sets cunter in shared memory
func (mtr *PbpbcinteccsidebandMetrics) SetRwrCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RwrCorrectable"))
	return nil
}

// SetRxdUncorrectable sets cunter in shared memory
func (mtr *PbpbcinteccsidebandMetrics) SetRxdUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxdUncorrectable"))
	return nil
}

// SetRxdCorrectable sets cunter in shared memory
func (mtr *PbpbcinteccsidebandMetrics) SetRxdCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxdCorrectable"))
	return nil
}

// PbpbcinteccsidebandMetricsIterator is the iterator object
type PbpbcinteccsidebandMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcinteccsidebandMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcinteccsidebandMetricsIterator) Next() *PbpbcinteccsidebandMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbcinteccsidebandMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcinteccsidebandMetricsIterator) Find(key uint64) (*PbpbcinteccsidebandMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcinteccsidebandMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcinteccsidebandMetricsIterator) Create(key uint64) (*PbpbcinteccsidebandMetrics, error) {
	tmtr := &PbpbcinteccsidebandMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcinteccsidebandMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcinteccsidebandMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbcinteccsidebandMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccsidebandMetricsIterator returns an iterator
func NewPbpbcinteccsidebandMetricsIterator() (*PbpbcinteccsidebandMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcinteccsidebandMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcinteccsidebandMetricsIterator{iter: iter}, nil
}

type PbpbcinteccncMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcinteccncMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcinteccncMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcinteccncMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcinteccncMetrics) getOffset(fldName string) int {
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
func (mtr *PbpbcinteccncMetrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *PbpbcinteccncMetrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// PbpbcinteccncMetricsIterator is the iterator object
type PbpbcinteccncMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcinteccncMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcinteccncMetricsIterator) Next() *PbpbcinteccncMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbcinteccncMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcinteccncMetricsIterator) Find(key uint64) (*PbpbcinteccncMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcinteccncMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcinteccncMetricsIterator) Create(key uint64) (*PbpbcinteccncMetrics, error) {
	tmtr := &PbpbcinteccncMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcinteccncMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcinteccncMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbcinteccncMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccncMetricsIterator returns an iterator
func NewPbpbcinteccncMetricsIterator() (*PbpbcinteccncMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcinteccncMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcinteccncMetricsIterator{iter: iter}, nil
}

type PbpbcinteccucMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcinteccucMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcinteccucMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcinteccucMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcinteccucMetrics) getOffset(fldName string) int {
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
func (mtr *PbpbcinteccucMetrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *PbpbcinteccucMetrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// PbpbcinteccucMetricsIterator is the iterator object
type PbpbcinteccucMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcinteccucMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcinteccucMetricsIterator) Next() *PbpbcinteccucMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbcinteccucMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcinteccucMetricsIterator) Find(key uint64) (*PbpbcinteccucMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcinteccucMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcinteccucMetricsIterator) Create(key uint64) (*PbpbcinteccucMetrics, error) {
	tmtr := &PbpbcinteccucMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcinteccucMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcinteccucMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbcinteccucMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccucMetricsIterator returns an iterator
func NewPbpbcinteccucMetricsIterator() (*PbpbcinteccucMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcinteccucMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcinteccucMetricsIterator{iter: iter}, nil
}

type PbpbcinteccportmoninMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcinteccportmoninMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcinteccportmoninMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcinteccportmoninMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcinteccportmoninMetrics) getOffset(fldName string) int {
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
func (mtr *PbpbcinteccportmoninMetrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *PbpbcinteccportmoninMetrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// PbpbcinteccportmoninMetricsIterator is the iterator object
type PbpbcinteccportmoninMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcinteccportmoninMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcinteccportmoninMetricsIterator) Next() *PbpbcinteccportmoninMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbcinteccportmoninMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcinteccportmoninMetricsIterator) Find(key uint64) (*PbpbcinteccportmoninMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcinteccportmoninMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcinteccportmoninMetricsIterator) Create(key uint64) (*PbpbcinteccportmoninMetrics, error) {
	tmtr := &PbpbcinteccportmoninMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcinteccportmoninMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcinteccportmoninMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbcinteccportmoninMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccportmoninMetricsIterator returns an iterator
func NewPbpbcinteccportmoninMetricsIterator() (*PbpbcinteccportmoninMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcinteccportmoninMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcinteccportmoninMetricsIterator{iter: iter}, nil
}

type PbpbcinteccportmonoutMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcinteccportmonoutMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcinteccportmonoutMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcinteccportmonoutMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcinteccportmonoutMetrics) getOffset(fldName string) int {
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
func (mtr *PbpbcinteccportmonoutMetrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *PbpbcinteccportmonoutMetrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// PbpbcinteccportmonoutMetricsIterator is the iterator object
type PbpbcinteccportmonoutMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcinteccportmonoutMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcinteccportmonoutMetricsIterator) Next() *PbpbcinteccportmonoutMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbcinteccportmonoutMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcinteccportmonoutMetricsIterator) Find(key uint64) (*PbpbcinteccportmonoutMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcinteccportmonoutMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcinteccportmonoutMetricsIterator) Create(key uint64) (*PbpbcinteccportmonoutMetrics, error) {
	tmtr := &PbpbcinteccportmonoutMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcinteccportmonoutMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcinteccportmonoutMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbcinteccportmonoutMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcinteccportmonoutMetricsIterator returns an iterator
func NewPbpbcinteccportmonoutMetricsIterator() (*PbpbcinteccportmonoutMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcinteccportmonoutMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcinteccportmonoutMetricsIterator{iter: iter}, nil
}

type PbpbcintrplMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MemoryError metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcintrplMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcintrplMetrics) Size() int {
	sz := 0

	sz += mtr.MemoryError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcintrplMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MemoryError = mtr.metrics.GetCounter(offset)
	offset += mtr.MemoryError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcintrplMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MemoryError" {
		return offset
	}
	offset += mtr.MemoryError.Size()

	return offset
}

// SetMemoryError sets cunter in shared memory
func (mtr *PbpbcintrplMetrics) SetMemoryError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MemoryError"))
	return nil
}

// PbpbcintrplMetricsIterator is the iterator object
type PbpbcintrplMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcintrplMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcintrplMetricsIterator) Next() *PbpbcintrplMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbcintrplMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcintrplMetricsIterator) Find(key uint64) (*PbpbcintrplMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcintrplMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcintrplMetricsIterator) Create(key uint64) (*PbpbcintrplMetrics, error) {
	tmtr := &PbpbcintrplMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcintrplMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcintrplMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbcintrplMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintrplMetricsIterator returns an iterator
func NewPbpbcintrplMetricsIterator() (*PbpbcintrplMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcintrplMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcintrplMetricsIterator{iter: iter}, nil
}

type PbpbcintcreditunderflowMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Port_10 metrics.Counter

	Port_11 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcintcreditunderflowMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcintcreditunderflowMetrics) Size() int {
	sz := 0

	sz += mtr.Port_10.Size()

	sz += mtr.Port_11.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcintcreditunderflowMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Port_10 = mtr.metrics.GetCounter(offset)
	offset += mtr.Port_10.Size()

	mtr.Port_11 = mtr.metrics.GetCounter(offset)
	offset += mtr.Port_11.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcintcreditunderflowMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Port_10" {
		return offset
	}
	offset += mtr.Port_10.Size()

	if fldName == "Port_11" {
		return offset
	}
	offset += mtr.Port_11.Size()

	return offset
}

// SetPort_10 sets cunter in shared memory
func (mtr *PbpbcintcreditunderflowMetrics) SetPort_10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Port_10"))
	return nil
}

// SetPort_11 sets cunter in shared memory
func (mtr *PbpbcintcreditunderflowMetrics) SetPort_11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Port_11"))
	return nil
}

// PbpbcintcreditunderflowMetricsIterator is the iterator object
type PbpbcintcreditunderflowMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcintcreditunderflowMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcintcreditunderflowMetricsIterator) Next() *PbpbcintcreditunderflowMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbcintcreditunderflowMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcintcreditunderflowMetricsIterator) Find(key uint64) (*PbpbcintcreditunderflowMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcintcreditunderflowMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcintcreditunderflowMetricsIterator) Create(key uint64) (*PbpbcintcreditunderflowMetrics, error) {
	tmtr := &PbpbcintcreditunderflowMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcintcreditunderflowMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcintcreditunderflowMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbcintcreditunderflowMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintcreditunderflowMetricsIterator returns an iterator
func NewPbpbcintcreditunderflowMetricsIterator() (*PbpbcintcreditunderflowMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcintcreditunderflowMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcintcreditunderflowMetricsIterator{iter: iter}, nil
}

type PbpbcintpbusviolationinMetrics struct {
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

	SopSopIn_10 metrics.Counter

	SopSopIn_11 metrics.Counter

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

	EopEopIn_10 metrics.Counter

	EopEopIn_11 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcintpbusviolationinMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcintpbusviolationinMetrics) Size() int {
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

	sz += mtr.SopSopIn_10.Size()

	sz += mtr.SopSopIn_11.Size()

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

	sz += mtr.EopEopIn_10.Size()

	sz += mtr.EopEopIn_11.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcintpbusviolationinMetrics) Unmarshal() error {
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

	mtr.SopSopIn_10 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_10.Size()

	mtr.SopSopIn_11 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_11.Size()

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

	mtr.EopEopIn_10 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_10.Size()

	mtr.EopEopIn_11 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_11.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) getOffset(fldName string) int {
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

	if fldName == "SopSopIn_10" {
		return offset
	}
	offset += mtr.SopSopIn_10.Size()

	if fldName == "SopSopIn_11" {
		return offset
	}
	offset += mtr.SopSopIn_11.Size()

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

	if fldName == "EopEopIn_10" {
		return offset
	}
	offset += mtr.EopEopIn_10.Size()

	if fldName == "EopEopIn_11" {
		return offset
	}
	offset += mtr.EopEopIn_11.Size()

	return offset
}

// SetSopSopIn_0 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetSopSopIn_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_0"))
	return nil
}

// SetSopSopIn_1 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetSopSopIn_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_1"))
	return nil
}

// SetSopSopIn_2 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetSopSopIn_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_2"))
	return nil
}

// SetSopSopIn_3 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetSopSopIn_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_3"))
	return nil
}

// SetSopSopIn_4 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetSopSopIn_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_4"))
	return nil
}

// SetSopSopIn_5 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetSopSopIn_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_5"))
	return nil
}

// SetSopSopIn_6 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetSopSopIn_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_6"))
	return nil
}

// SetSopSopIn_7 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetSopSopIn_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_7"))
	return nil
}

// SetSopSopIn_8 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetSopSopIn_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_8"))
	return nil
}

// SetSopSopIn_9 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetSopSopIn_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_9"))
	return nil
}

// SetSopSopIn_10 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetSopSopIn_10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_10"))
	return nil
}

// SetSopSopIn_11 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetSopSopIn_11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_11"))
	return nil
}

// SetEopEopIn_0 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetEopEopIn_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_0"))
	return nil
}

// SetEopEopIn_1 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetEopEopIn_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_1"))
	return nil
}

// SetEopEopIn_2 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetEopEopIn_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_2"))
	return nil
}

// SetEopEopIn_3 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetEopEopIn_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_3"))
	return nil
}

// SetEopEopIn_4 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetEopEopIn_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_4"))
	return nil
}

// SetEopEopIn_5 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetEopEopIn_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_5"))
	return nil
}

// SetEopEopIn_6 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetEopEopIn_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_6"))
	return nil
}

// SetEopEopIn_7 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetEopEopIn_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_7"))
	return nil
}

// SetEopEopIn_8 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetEopEopIn_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_8"))
	return nil
}

// SetEopEopIn_9 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetEopEopIn_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_9"))
	return nil
}

// SetEopEopIn_10 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetEopEopIn_10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_10"))
	return nil
}

// SetEopEopIn_11 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationinMetrics) SetEopEopIn_11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_11"))
	return nil
}

// PbpbcintpbusviolationinMetricsIterator is the iterator object
type PbpbcintpbusviolationinMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcintpbusviolationinMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcintpbusviolationinMetricsIterator) Next() *PbpbcintpbusviolationinMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbcintpbusviolationinMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcintpbusviolationinMetricsIterator) Find(key uint64) (*PbpbcintpbusviolationinMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcintpbusviolationinMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcintpbusviolationinMetricsIterator) Create(key uint64) (*PbpbcintpbusviolationinMetrics, error) {
	tmtr := &PbpbcintpbusviolationinMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcintpbusviolationinMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcintpbusviolationinMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbcintpbusviolationinMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintpbusviolationinMetricsIterator returns an iterator
func NewPbpbcintpbusviolationinMetricsIterator() (*PbpbcintpbusviolationinMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcintpbusviolationinMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcintpbusviolationinMetricsIterator{iter: iter}, nil
}

type PbpbcintpbusviolationoutMetrics struct {
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

	SopSopOut_10 metrics.Counter

	SopSopOut_11 metrics.Counter

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

	EopEopOut_10 metrics.Counter

	EopEopOut_11 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcintpbusviolationoutMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcintpbusviolationoutMetrics) Size() int {
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

	sz += mtr.SopSopOut_10.Size()

	sz += mtr.SopSopOut_11.Size()

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

	sz += mtr.EopEopOut_10.Size()

	sz += mtr.EopEopOut_11.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) Unmarshal() error {
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

	mtr.SopSopOut_10 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopOut_10.Size()

	mtr.SopSopOut_11 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopOut_11.Size()

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

	mtr.EopEopOut_10 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopOut_10.Size()

	mtr.EopEopOut_11 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopOut_11.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) getOffset(fldName string) int {
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

	if fldName == "SopSopOut_10" {
		return offset
	}
	offset += mtr.SopSopOut_10.Size()

	if fldName == "SopSopOut_11" {
		return offset
	}
	offset += mtr.SopSopOut_11.Size()

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

	if fldName == "EopEopOut_10" {
		return offset
	}
	offset += mtr.EopEopOut_10.Size()

	if fldName == "EopEopOut_11" {
		return offset
	}
	offset += mtr.EopEopOut_11.Size()

	return offset
}

// SetSopSopOut_0 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetSopSopOut_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_0"))
	return nil
}

// SetSopSopOut_1 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetSopSopOut_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_1"))
	return nil
}

// SetSopSopOut_2 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetSopSopOut_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_2"))
	return nil
}

// SetSopSopOut_3 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetSopSopOut_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_3"))
	return nil
}

// SetSopSopOut_4 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetSopSopOut_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_4"))
	return nil
}

// SetSopSopOut_5 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetSopSopOut_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_5"))
	return nil
}

// SetSopSopOut_6 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetSopSopOut_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_6"))
	return nil
}

// SetSopSopOut_7 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetSopSopOut_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_7"))
	return nil
}

// SetSopSopOut_8 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetSopSopOut_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_8"))
	return nil
}

// SetSopSopOut_9 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetSopSopOut_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_9"))
	return nil
}

// SetSopSopOut_10 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetSopSopOut_10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_10"))
	return nil
}

// SetSopSopOut_11 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetSopSopOut_11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopOut_11"))
	return nil
}

// SetEopEopOut_0 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetEopEopOut_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_0"))
	return nil
}

// SetEopEopOut_1 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetEopEopOut_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_1"))
	return nil
}

// SetEopEopOut_2 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetEopEopOut_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_2"))
	return nil
}

// SetEopEopOut_3 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetEopEopOut_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_3"))
	return nil
}

// SetEopEopOut_4 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetEopEopOut_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_4"))
	return nil
}

// SetEopEopOut_5 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetEopEopOut_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_5"))
	return nil
}

// SetEopEopOut_6 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetEopEopOut_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_6"))
	return nil
}

// SetEopEopOut_7 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetEopEopOut_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_7"))
	return nil
}

// SetEopEopOut_8 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetEopEopOut_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_8"))
	return nil
}

// SetEopEopOut_9 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetEopEopOut_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_9"))
	return nil
}

// SetEopEopOut_10 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetEopEopOut_10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_10"))
	return nil
}

// SetEopEopOut_11 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationoutMetrics) SetEopEopOut_11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopOut_11"))
	return nil
}

// PbpbcintpbusviolationoutMetricsIterator is the iterator object
type PbpbcintpbusviolationoutMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcintpbusviolationoutMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcintpbusviolationoutMetricsIterator) Next() *PbpbcintpbusviolationoutMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbcintpbusviolationoutMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcintpbusviolationoutMetricsIterator) Find(key uint64) (*PbpbcintpbusviolationoutMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcintpbusviolationoutMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcintpbusviolationoutMetricsIterator) Create(key uint64) (*PbpbcintpbusviolationoutMetrics, error) {
	tmtr := &PbpbcintpbusviolationoutMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcintpbusviolationoutMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcintpbusviolationoutMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbcintpbusviolationoutMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbcintpbusviolationoutMetricsIterator returns an iterator
func NewPbpbcintpbusviolationoutMetricsIterator() (*PbpbcintpbusviolationoutMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcintpbusviolationoutMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcintpbusviolationoutMetricsIterator{iter: iter}, nil
}

type PbpbchbmintecchbmmtuMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbmintecchbmmtuMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbmintecchbmmtuMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbmintecchbmmtuMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbmintecchbmmtuMetrics) getOffset(fldName string) int {
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
func (mtr *PbpbchbmintecchbmmtuMetrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *PbpbchbmintecchbmmtuMetrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// PbpbchbmintecchbmmtuMetricsIterator is the iterator object
type PbpbchbmintecchbmmtuMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbmintecchbmmtuMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbmintecchbmmtuMetricsIterator) Next() *PbpbchbmintecchbmmtuMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbchbmintecchbmmtuMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbmintecchbmmtuMetricsIterator) Find(key uint64) (*PbpbchbmintecchbmmtuMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbmintecchbmmtuMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbmintecchbmmtuMetricsIterator) Create(key uint64) (*PbpbchbmintecchbmmtuMetrics, error) {
	tmtr := &PbpbchbmintecchbmmtuMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbmintecchbmmtuMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbmintecchbmmtuMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbchbmintecchbmmtuMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbchbmintecchbmmtuMetricsIterator returns an iterator
func NewPbpbchbmintecchbmmtuMetricsIterator() (*PbpbchbmintecchbmmtuMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbmintecchbmmtuMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbmintecchbmmtuMetricsIterator{iter: iter}, nil
}

type PbpbchbmintecchbmrbMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RbUncorrectable metrics.Counter

	RbCorrectable metrics.Counter

	CdtUncorrectable metrics.Counter

	CdtCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbmintecchbmrbMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbmintecchbmrbMetrics) Size() int {
	sz := 0

	sz += mtr.RbUncorrectable.Size()

	sz += mtr.RbCorrectable.Size()

	sz += mtr.CdtUncorrectable.Size()

	sz += mtr.CdtCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbmintecchbmrbMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RbUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RbUncorrectable.Size()

	mtr.RbCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RbCorrectable.Size()

	mtr.CdtUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.CdtUncorrectable.Size()

	mtr.CdtCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.CdtCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbmintecchbmrbMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RbUncorrectable" {
		return offset
	}
	offset += mtr.RbUncorrectable.Size()

	if fldName == "RbCorrectable" {
		return offset
	}
	offset += mtr.RbCorrectable.Size()

	if fldName == "CdtUncorrectable" {
		return offset
	}
	offset += mtr.CdtUncorrectable.Size()

	if fldName == "CdtCorrectable" {
		return offset
	}
	offset += mtr.CdtCorrectable.Size()

	return offset
}

// SetRbUncorrectable sets cunter in shared memory
func (mtr *PbpbchbmintecchbmrbMetrics) SetRbUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RbUncorrectable"))
	return nil
}

// SetRbCorrectable sets cunter in shared memory
func (mtr *PbpbchbmintecchbmrbMetrics) SetRbCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RbCorrectable"))
	return nil
}

// SetCdtUncorrectable sets cunter in shared memory
func (mtr *PbpbchbmintecchbmrbMetrics) SetCdtUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CdtUncorrectable"))
	return nil
}

// SetCdtCorrectable sets cunter in shared memory
func (mtr *PbpbchbmintecchbmrbMetrics) SetCdtCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CdtCorrectable"))
	return nil
}

// PbpbchbmintecchbmrbMetricsIterator is the iterator object
type PbpbchbmintecchbmrbMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbmintecchbmrbMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbmintecchbmrbMetricsIterator) Next() *PbpbchbmintecchbmrbMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbchbmintecchbmrbMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbmintecchbmrbMetricsIterator) Find(key uint64) (*PbpbchbmintecchbmrbMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbmintecchbmrbMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbmintecchbmrbMetricsIterator) Create(key uint64) (*PbpbchbmintecchbmrbMetrics, error) {
	tmtr := &PbpbchbmintecchbmrbMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbmintecchbmrbMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbmintecchbmrbMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbchbmintecchbmrbMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbchbmintecchbmrbMetricsIterator returns an iterator
func NewPbpbchbmintecchbmrbMetricsIterator() (*PbpbchbmintecchbmrbMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbmintecchbmrbMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbmintecchbmrbMetricsIterator{iter: iter}, nil
}

type PbpbchbmintecchbmwbMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbmintecchbmwbMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbmintecchbmwbMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbmintecchbmwbMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbmintecchbmwbMetrics) getOffset(fldName string) int {
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
func (mtr *PbpbchbmintecchbmwbMetrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *PbpbchbmintecchbmwbMetrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// PbpbchbmintecchbmwbMetricsIterator is the iterator object
type PbpbchbmintecchbmwbMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbmintecchbmwbMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbmintecchbmwbMetricsIterator) Next() *PbpbchbmintecchbmwbMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &PbpbchbmintecchbmwbMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbmintecchbmwbMetricsIterator) Find(key uint64) (*PbpbchbmintecchbmwbMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbmintecchbmwbMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbmintecchbmwbMetricsIterator) Create(key uint64) (*PbpbchbmintecchbmwbMetrics, error) {
	tmtr := &PbpbchbmintecchbmwbMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbmintecchbmwbMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbmintecchbmwbMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *PbpbchbmintecchbmwbMetricsIterator) Free() {
	it.iter.Free()
}

// NewPbpbchbmintecchbmwbMetricsIterator returns an iterator
func NewPbpbchbmintecchbmwbMetricsIterator() (*PbpbchbmintecchbmwbMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbmintecchbmwbMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbmintecchbmwbMetricsIterator{iter: iter}, nil
}

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

	BxpbPbusDrdy metrics.Counter

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

	sz += mtr.BxpbPbusDrdy.Size()

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

	mtr.BxpbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.BxpbPbusDrdy.Size()

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

	if fldName == "BxpbPbusDrdy" {
		return offset
	}
	offset += mtr.BxpbPbusDrdy.Size()

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

// SetBxpbPbusDrdy sets cunter in shared memory
func (mtr *BxbxintmacMetrics) SetBxpbPbusDrdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BxpbPbusDrdy"))
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

	ErrPtrFfOverflow metrics.Counter

	ErrCsumFfOverflow metrics.Counter

	ErrPktoutFfOverflow metrics.Counter

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

	sz += mtr.ErrPtrFfOverflow.Size()

	sz += mtr.ErrCsumFfOverflow.Size()

	sz += mtr.ErrPktoutFfOverflow.Size()

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

	mtr.ErrPtrFfOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPtrFfOverflow.Size()

	mtr.ErrCsumFfOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumFfOverflow.Size()

	mtr.ErrPktoutFfOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPktoutFfOverflow.Size()

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
