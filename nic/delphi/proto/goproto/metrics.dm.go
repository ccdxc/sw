// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/ntsdb/metrics"
)

type LifMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RxUnicastBytes metrics.Counter

	RxUnicastPackets metrics.Counter

	RxMulticastBytes metrics.Counter

	RxMulticastPackets metrics.Counter

	RxBroadcastBytes metrics.Counter

	RxBroadcastPackets metrics.Counter

	XRsvd__0 metrics.Counter

	XRsvd__1 metrics.Counter

	RxDropUnicastBytes metrics.Counter

	RxDropUnicastPackets metrics.Counter

	RxDropMulticastBytes metrics.Counter

	RxDropMulticastPackets metrics.Counter

	RxDropBroadcastBytes metrics.Counter

	RxDropBroadcastPackets metrics.Counter

	RxDmaError metrics.Counter

	XRsvd__2 metrics.Counter

	TxUnicastBytes metrics.Counter

	TxUnicastPackets metrics.Counter

	TxMulticastBytes metrics.Counter

	TxMulticastPackets metrics.Counter

	TxBroadcastBytes metrics.Counter

	TxBroadcastPackets metrics.Counter

	XRsvd__3 metrics.Counter

	XRsvd__4 metrics.Counter

	TxDropUnicastBytes metrics.Counter

	TxDropUnicastPackets metrics.Counter

	TxDropMulticastBytes metrics.Counter

	TxDropMulticastPackets metrics.Counter

	TxDropBroadcastBytes metrics.Counter

	TxDropBroadcastPackets metrics.Counter

	TxDmaError metrics.Counter

	XRsvd__5 metrics.Counter

	RxQueueDisabledDrops metrics.Counter

	RxQueueEmptyDrops metrics.Counter

	RxQueueSched metrics.Counter

	RxDescFetchError metrics.Counter

	RxDescDataError metrics.Counter

	XRsvd__6 metrics.Counter

	XRsvd__7 metrics.Counter

	XRsvd__8 metrics.Counter

	TxQueueDisabledDrops metrics.Counter

	TxQueueSched metrics.Counter

	TxDescFetchError metrics.Counter

	TxDescDataError metrics.Counter

	XRsvd__9 metrics.Counter

	XRsvd__10 metrics.Counter

	XRsvd__11 metrics.Counter

	XRsvd__12 metrics.Counter

	RxRss metrics.Counter

	RxCsumComplete metrics.Counter

	RxIpBad metrics.Counter

	RxTcpBad metrics.Counter

	RxUdpBad metrics.Counter

	RxVlanStrip metrics.Counter

	XRsvd__13 metrics.Counter

	XRsvd__14 metrics.Counter

	TxCsumHw metrics.Counter

	TxCsumHwInner metrics.Counter

	TxVlanInsert metrics.Counter

	TxSg metrics.Counter

	TxTsoSg metrics.Counter

	TxTsoSop metrics.Counter

	TxTsoEop metrics.Counter

	XRsvd__15 metrics.Counter

	TxDescOpcInvalid metrics.Counter

	TxDescOpcCsumNone metrics.Counter

	TxDescOpcCsumPartial metrics.Counter

	TxDescOpcCsumHw metrics.Counter

	TxDescOpcCsumTso metrics.Counter

	XRsvd__16 metrics.Counter

	XRsvd__17 metrics.Counter

	XRsvd__18 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *LifMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *LifMetrics) Size() int {
	sz := 0

	sz += mtr.RxUnicastBytes.Size()

	sz += mtr.RxUnicastPackets.Size()

	sz += mtr.RxMulticastBytes.Size()

	sz += mtr.RxMulticastPackets.Size()

	sz += mtr.RxBroadcastBytes.Size()

	sz += mtr.RxBroadcastPackets.Size()

	sz += mtr.XRsvd__0.Size()

	sz += mtr.XRsvd__1.Size()

	sz += mtr.RxDropUnicastBytes.Size()

	sz += mtr.RxDropUnicastPackets.Size()

	sz += mtr.RxDropMulticastBytes.Size()

	sz += mtr.RxDropMulticastPackets.Size()

	sz += mtr.RxDropBroadcastBytes.Size()

	sz += mtr.RxDropBroadcastPackets.Size()

	sz += mtr.RxDmaError.Size()

	sz += mtr.XRsvd__2.Size()

	sz += mtr.TxUnicastBytes.Size()

	sz += mtr.TxUnicastPackets.Size()

	sz += mtr.TxMulticastBytes.Size()

	sz += mtr.TxMulticastPackets.Size()

	sz += mtr.TxBroadcastBytes.Size()

	sz += mtr.TxBroadcastPackets.Size()

	sz += mtr.XRsvd__3.Size()

	sz += mtr.XRsvd__4.Size()

	sz += mtr.TxDropUnicastBytes.Size()

	sz += mtr.TxDropUnicastPackets.Size()

	sz += mtr.TxDropMulticastBytes.Size()

	sz += mtr.TxDropMulticastPackets.Size()

	sz += mtr.TxDropBroadcastBytes.Size()

	sz += mtr.TxDropBroadcastPackets.Size()

	sz += mtr.TxDmaError.Size()

	sz += mtr.XRsvd__5.Size()

	sz += mtr.RxQueueDisabledDrops.Size()

	sz += mtr.RxQueueEmptyDrops.Size()

	sz += mtr.RxQueueSched.Size()

	sz += mtr.RxDescFetchError.Size()

	sz += mtr.RxDescDataError.Size()

	sz += mtr.XRsvd__6.Size()

	sz += mtr.XRsvd__7.Size()

	sz += mtr.XRsvd__8.Size()

	sz += mtr.TxQueueDisabledDrops.Size()

	sz += mtr.TxQueueSched.Size()

	sz += mtr.TxDescFetchError.Size()

	sz += mtr.TxDescDataError.Size()

	sz += mtr.XRsvd__9.Size()

	sz += mtr.XRsvd__10.Size()

	sz += mtr.XRsvd__11.Size()

	sz += mtr.XRsvd__12.Size()

	sz += mtr.RxRss.Size()

	sz += mtr.RxCsumComplete.Size()

	sz += mtr.RxIpBad.Size()

	sz += mtr.RxTcpBad.Size()

	sz += mtr.RxUdpBad.Size()

	sz += mtr.RxVlanStrip.Size()

	sz += mtr.XRsvd__13.Size()

	sz += mtr.XRsvd__14.Size()

	sz += mtr.TxCsumHw.Size()

	sz += mtr.TxCsumHwInner.Size()

	sz += mtr.TxVlanInsert.Size()

	sz += mtr.TxSg.Size()

	sz += mtr.TxTsoSg.Size()

	sz += mtr.TxTsoSop.Size()

	sz += mtr.TxTsoEop.Size()

	sz += mtr.XRsvd__15.Size()

	sz += mtr.TxDescOpcInvalid.Size()

	sz += mtr.TxDescOpcCsumNone.Size()

	sz += mtr.TxDescOpcCsumPartial.Size()

	sz += mtr.TxDescOpcCsumHw.Size()

	sz += mtr.TxDescOpcCsumTso.Size()

	sz += mtr.XRsvd__16.Size()

	sz += mtr.XRsvd__17.Size()

	sz += mtr.XRsvd__18.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *LifMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RxUnicastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.RxUnicastBytes.Size()

	mtr.RxUnicastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RxUnicastPackets.Size()

	mtr.RxMulticastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.RxMulticastBytes.Size()

	mtr.RxMulticastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RxMulticastPackets.Size()

	mtr.RxBroadcastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.RxBroadcastBytes.Size()

	mtr.RxBroadcastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RxBroadcastPackets.Size()

	mtr.XRsvd__0 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__0.Size()

	mtr.XRsvd__1 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__1.Size()

	mtr.RxDropUnicastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.RxDropUnicastBytes.Size()

	mtr.RxDropUnicastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RxDropUnicastPackets.Size()

	mtr.RxDropMulticastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.RxDropMulticastBytes.Size()

	mtr.RxDropMulticastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RxDropMulticastPackets.Size()

	mtr.RxDropBroadcastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.RxDropBroadcastBytes.Size()

	mtr.RxDropBroadcastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RxDropBroadcastPackets.Size()

	mtr.RxDmaError = mtr.metrics.GetCounter(offset)
	offset += mtr.RxDmaError.Size()

	mtr.XRsvd__2 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__2.Size()

	mtr.TxUnicastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxUnicastBytes.Size()

	mtr.TxUnicastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxUnicastPackets.Size()

	mtr.TxMulticastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxMulticastBytes.Size()

	mtr.TxMulticastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxMulticastPackets.Size()

	mtr.TxBroadcastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxBroadcastBytes.Size()

	mtr.TxBroadcastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxBroadcastPackets.Size()

	mtr.XRsvd__3 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__3.Size()

	mtr.XRsvd__4 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__4.Size()

	mtr.TxDropUnicastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDropUnicastBytes.Size()

	mtr.TxDropUnicastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDropUnicastPackets.Size()

	mtr.TxDropMulticastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDropMulticastBytes.Size()

	mtr.TxDropMulticastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDropMulticastPackets.Size()

	mtr.TxDropBroadcastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDropBroadcastBytes.Size()

	mtr.TxDropBroadcastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDropBroadcastPackets.Size()

	mtr.TxDmaError = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDmaError.Size()

	mtr.XRsvd__5 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__5.Size()

	mtr.RxQueueDisabledDrops = mtr.metrics.GetCounter(offset)
	offset += mtr.RxQueueDisabledDrops.Size()

	mtr.RxQueueEmptyDrops = mtr.metrics.GetCounter(offset)
	offset += mtr.RxQueueEmptyDrops.Size()

	mtr.RxQueueSched = mtr.metrics.GetCounter(offset)
	offset += mtr.RxQueueSched.Size()

	mtr.RxDescFetchError = mtr.metrics.GetCounter(offset)
	offset += mtr.RxDescFetchError.Size()

	mtr.RxDescDataError = mtr.metrics.GetCounter(offset)
	offset += mtr.RxDescDataError.Size()

	mtr.XRsvd__6 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__6.Size()

	mtr.XRsvd__7 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__7.Size()

	mtr.XRsvd__8 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__8.Size()

	mtr.TxQueueDisabledDrops = mtr.metrics.GetCounter(offset)
	offset += mtr.TxQueueDisabledDrops.Size()

	mtr.TxQueueSched = mtr.metrics.GetCounter(offset)
	offset += mtr.TxQueueSched.Size()

	mtr.TxDescFetchError = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDescFetchError.Size()

	mtr.TxDescDataError = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDescDataError.Size()

	mtr.XRsvd__9 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__9.Size()

	mtr.XRsvd__10 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__10.Size()

	mtr.XRsvd__11 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__11.Size()

	mtr.XRsvd__12 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__12.Size()

	mtr.RxRss = mtr.metrics.GetCounter(offset)
	offset += mtr.RxRss.Size()

	mtr.RxCsumComplete = mtr.metrics.GetCounter(offset)
	offset += mtr.RxCsumComplete.Size()

	mtr.RxIpBad = mtr.metrics.GetCounter(offset)
	offset += mtr.RxIpBad.Size()

	mtr.RxTcpBad = mtr.metrics.GetCounter(offset)
	offset += mtr.RxTcpBad.Size()

	mtr.RxUdpBad = mtr.metrics.GetCounter(offset)
	offset += mtr.RxUdpBad.Size()

	mtr.RxVlanStrip = mtr.metrics.GetCounter(offset)
	offset += mtr.RxVlanStrip.Size()

	mtr.XRsvd__13 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__13.Size()

	mtr.XRsvd__14 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__14.Size()

	mtr.TxCsumHw = mtr.metrics.GetCounter(offset)
	offset += mtr.TxCsumHw.Size()

	mtr.TxCsumHwInner = mtr.metrics.GetCounter(offset)
	offset += mtr.TxCsumHwInner.Size()

	mtr.TxVlanInsert = mtr.metrics.GetCounter(offset)
	offset += mtr.TxVlanInsert.Size()

	mtr.TxSg = mtr.metrics.GetCounter(offset)
	offset += mtr.TxSg.Size()

	mtr.TxTsoSg = mtr.metrics.GetCounter(offset)
	offset += mtr.TxTsoSg.Size()

	mtr.TxTsoSop = mtr.metrics.GetCounter(offset)
	offset += mtr.TxTsoSop.Size()

	mtr.TxTsoEop = mtr.metrics.GetCounter(offset)
	offset += mtr.TxTsoEop.Size()

	mtr.XRsvd__15 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__15.Size()

	mtr.TxDescOpcInvalid = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDescOpcInvalid.Size()

	mtr.TxDescOpcCsumNone = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDescOpcCsumNone.Size()

	mtr.TxDescOpcCsumPartial = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDescOpcCsumPartial.Size()

	mtr.TxDescOpcCsumHw = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDescOpcCsumHw.Size()

	mtr.TxDescOpcCsumTso = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDescOpcCsumTso.Size()

	mtr.XRsvd__16 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__16.Size()

	mtr.XRsvd__17 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__17.Size()

	mtr.XRsvd__18 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__18.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *LifMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxUnicastBytes" {
		return offset
	}
	offset += mtr.RxUnicastBytes.Size()

	if fldName == "RxUnicastPackets" {
		return offset
	}
	offset += mtr.RxUnicastPackets.Size()

	if fldName == "RxMulticastBytes" {
		return offset
	}
	offset += mtr.RxMulticastBytes.Size()

	if fldName == "RxMulticastPackets" {
		return offset
	}
	offset += mtr.RxMulticastPackets.Size()

	if fldName == "RxBroadcastBytes" {
		return offset
	}
	offset += mtr.RxBroadcastBytes.Size()

	if fldName == "RxBroadcastPackets" {
		return offset
	}
	offset += mtr.RxBroadcastPackets.Size()

	if fldName == "XRsvd__0" {
		return offset
	}
	offset += mtr.XRsvd__0.Size()

	if fldName == "XRsvd__1" {
		return offset
	}
	offset += mtr.XRsvd__1.Size()

	if fldName == "RxDropUnicastBytes" {
		return offset
	}
	offset += mtr.RxDropUnicastBytes.Size()

	if fldName == "RxDropUnicastPackets" {
		return offset
	}
	offset += mtr.RxDropUnicastPackets.Size()

	if fldName == "RxDropMulticastBytes" {
		return offset
	}
	offset += mtr.RxDropMulticastBytes.Size()

	if fldName == "RxDropMulticastPackets" {
		return offset
	}
	offset += mtr.RxDropMulticastPackets.Size()

	if fldName == "RxDropBroadcastBytes" {
		return offset
	}
	offset += mtr.RxDropBroadcastBytes.Size()

	if fldName == "RxDropBroadcastPackets" {
		return offset
	}
	offset += mtr.RxDropBroadcastPackets.Size()

	if fldName == "RxDmaError" {
		return offset
	}
	offset += mtr.RxDmaError.Size()

	if fldName == "XRsvd__2" {
		return offset
	}
	offset += mtr.XRsvd__2.Size()

	if fldName == "TxUnicastBytes" {
		return offset
	}
	offset += mtr.TxUnicastBytes.Size()

	if fldName == "TxUnicastPackets" {
		return offset
	}
	offset += mtr.TxUnicastPackets.Size()

	if fldName == "TxMulticastBytes" {
		return offset
	}
	offset += mtr.TxMulticastBytes.Size()

	if fldName == "TxMulticastPackets" {
		return offset
	}
	offset += mtr.TxMulticastPackets.Size()

	if fldName == "TxBroadcastBytes" {
		return offset
	}
	offset += mtr.TxBroadcastBytes.Size()

	if fldName == "TxBroadcastPackets" {
		return offset
	}
	offset += mtr.TxBroadcastPackets.Size()

	if fldName == "XRsvd__3" {
		return offset
	}
	offset += mtr.XRsvd__3.Size()

	if fldName == "XRsvd__4" {
		return offset
	}
	offset += mtr.XRsvd__4.Size()

	if fldName == "TxDropUnicastBytes" {
		return offset
	}
	offset += mtr.TxDropUnicastBytes.Size()

	if fldName == "TxDropUnicastPackets" {
		return offset
	}
	offset += mtr.TxDropUnicastPackets.Size()

	if fldName == "TxDropMulticastBytes" {
		return offset
	}
	offset += mtr.TxDropMulticastBytes.Size()

	if fldName == "TxDropMulticastPackets" {
		return offset
	}
	offset += mtr.TxDropMulticastPackets.Size()

	if fldName == "TxDropBroadcastBytes" {
		return offset
	}
	offset += mtr.TxDropBroadcastBytes.Size()

	if fldName == "TxDropBroadcastPackets" {
		return offset
	}
	offset += mtr.TxDropBroadcastPackets.Size()

	if fldName == "TxDmaError" {
		return offset
	}
	offset += mtr.TxDmaError.Size()

	if fldName == "XRsvd__5" {
		return offset
	}
	offset += mtr.XRsvd__5.Size()

	if fldName == "RxQueueDisabledDrops" {
		return offset
	}
	offset += mtr.RxQueueDisabledDrops.Size()

	if fldName == "RxQueueEmptyDrops" {
		return offset
	}
	offset += mtr.RxQueueEmptyDrops.Size()

	if fldName == "RxQueueSched" {
		return offset
	}
	offset += mtr.RxQueueSched.Size()

	if fldName == "RxDescFetchError" {
		return offset
	}
	offset += mtr.RxDescFetchError.Size()

	if fldName == "RxDescDataError" {
		return offset
	}
	offset += mtr.RxDescDataError.Size()

	if fldName == "XRsvd__6" {
		return offset
	}
	offset += mtr.XRsvd__6.Size()

	if fldName == "XRsvd__7" {
		return offset
	}
	offset += mtr.XRsvd__7.Size()

	if fldName == "XRsvd__8" {
		return offset
	}
	offset += mtr.XRsvd__8.Size()

	if fldName == "TxQueueDisabledDrops" {
		return offset
	}
	offset += mtr.TxQueueDisabledDrops.Size()

	if fldName == "TxQueueSched" {
		return offset
	}
	offset += mtr.TxQueueSched.Size()

	if fldName == "TxDescFetchError" {
		return offset
	}
	offset += mtr.TxDescFetchError.Size()

	if fldName == "TxDescDataError" {
		return offset
	}
	offset += mtr.TxDescDataError.Size()

	if fldName == "XRsvd__9" {
		return offset
	}
	offset += mtr.XRsvd__9.Size()

	if fldName == "XRsvd__10" {
		return offset
	}
	offset += mtr.XRsvd__10.Size()

	if fldName == "XRsvd__11" {
		return offset
	}
	offset += mtr.XRsvd__11.Size()

	if fldName == "XRsvd__12" {
		return offset
	}
	offset += mtr.XRsvd__12.Size()

	if fldName == "RxRss" {
		return offset
	}
	offset += mtr.RxRss.Size()

	if fldName == "RxCsumComplete" {
		return offset
	}
	offset += mtr.RxCsumComplete.Size()

	if fldName == "RxIpBad" {
		return offset
	}
	offset += mtr.RxIpBad.Size()

	if fldName == "RxTcpBad" {
		return offset
	}
	offset += mtr.RxTcpBad.Size()

	if fldName == "RxUdpBad" {
		return offset
	}
	offset += mtr.RxUdpBad.Size()

	if fldName == "RxVlanStrip" {
		return offset
	}
	offset += mtr.RxVlanStrip.Size()

	if fldName == "XRsvd__13" {
		return offset
	}
	offset += mtr.XRsvd__13.Size()

	if fldName == "XRsvd__14" {
		return offset
	}
	offset += mtr.XRsvd__14.Size()

	if fldName == "TxCsumHw" {
		return offset
	}
	offset += mtr.TxCsumHw.Size()

	if fldName == "TxCsumHwInner" {
		return offset
	}
	offset += mtr.TxCsumHwInner.Size()

	if fldName == "TxVlanInsert" {
		return offset
	}
	offset += mtr.TxVlanInsert.Size()

	if fldName == "TxSg" {
		return offset
	}
	offset += mtr.TxSg.Size()

	if fldName == "TxTsoSg" {
		return offset
	}
	offset += mtr.TxTsoSg.Size()

	if fldName == "TxTsoSop" {
		return offset
	}
	offset += mtr.TxTsoSop.Size()

	if fldName == "TxTsoEop" {
		return offset
	}
	offset += mtr.TxTsoEop.Size()

	if fldName == "XRsvd__15" {
		return offset
	}
	offset += mtr.XRsvd__15.Size()

	if fldName == "TxDescOpcInvalid" {
		return offset
	}
	offset += mtr.TxDescOpcInvalid.Size()

	if fldName == "TxDescOpcCsumNone" {
		return offset
	}
	offset += mtr.TxDescOpcCsumNone.Size()

	if fldName == "TxDescOpcCsumPartial" {
		return offset
	}
	offset += mtr.TxDescOpcCsumPartial.Size()

	if fldName == "TxDescOpcCsumHw" {
		return offset
	}
	offset += mtr.TxDescOpcCsumHw.Size()

	if fldName == "TxDescOpcCsumTso" {
		return offset
	}
	offset += mtr.TxDescOpcCsumTso.Size()

	if fldName == "XRsvd__16" {
		return offset
	}
	offset += mtr.XRsvd__16.Size()

	if fldName == "XRsvd__17" {
		return offset
	}
	offset += mtr.XRsvd__17.Size()

	if fldName == "XRsvd__18" {
		return offset
	}
	offset += mtr.XRsvd__18.Size()

	return offset
}

// SetRxUnicastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetRxUnicastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxUnicastBytes"))
	return nil
}

// SetRxUnicastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRxUnicastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxUnicastPackets"))
	return nil
}

// SetRxMulticastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetRxMulticastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxMulticastBytes"))
	return nil
}

// SetRxMulticastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRxMulticastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxMulticastPackets"))
	return nil
}

// SetRxBroadcastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetRxBroadcastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxBroadcastBytes"))
	return nil
}

// SetRxBroadcastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRxBroadcastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxBroadcastPackets"))
	return nil
}

// SetXRsvd__0 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__0"))
	return nil
}

// SetXRsvd__1 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__1"))
	return nil
}

// SetRxDropUnicastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetRxDropUnicastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxDropUnicastBytes"))
	return nil
}

// SetRxDropUnicastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRxDropUnicastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxDropUnicastPackets"))
	return nil
}

// SetRxDropMulticastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetRxDropMulticastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxDropMulticastBytes"))
	return nil
}

// SetRxDropMulticastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRxDropMulticastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxDropMulticastPackets"))
	return nil
}

// SetRxDropBroadcastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetRxDropBroadcastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxDropBroadcastBytes"))
	return nil
}

// SetRxDropBroadcastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRxDropBroadcastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxDropBroadcastPackets"))
	return nil
}

// SetRxDmaError sets cunter in shared memory
func (mtr *LifMetrics) SetRxDmaError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxDmaError"))
	return nil
}

// SetXRsvd__2 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__2"))
	return nil
}

// SetTxUnicastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxUnicastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxUnicastBytes"))
	return nil
}

// SetTxUnicastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxUnicastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxUnicastPackets"))
	return nil
}

// SetTxMulticastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxMulticastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxMulticastBytes"))
	return nil
}

// SetTxMulticastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxMulticastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxMulticastPackets"))
	return nil
}

// SetTxBroadcastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxBroadcastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxBroadcastBytes"))
	return nil
}

// SetTxBroadcastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxBroadcastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxBroadcastPackets"))
	return nil
}

// SetXRsvd__3 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__3"))
	return nil
}

// SetXRsvd__4 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__4"))
	return nil
}

// SetTxDropUnicastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxDropUnicastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDropUnicastBytes"))
	return nil
}

// SetTxDropUnicastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxDropUnicastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDropUnicastPackets"))
	return nil
}

// SetTxDropMulticastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxDropMulticastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDropMulticastBytes"))
	return nil
}

// SetTxDropMulticastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxDropMulticastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDropMulticastPackets"))
	return nil
}

// SetTxDropBroadcastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxDropBroadcastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDropBroadcastBytes"))
	return nil
}

// SetTxDropBroadcastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxDropBroadcastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDropBroadcastPackets"))
	return nil
}

// SetTxDmaError sets cunter in shared memory
func (mtr *LifMetrics) SetTxDmaError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDmaError"))
	return nil
}

// SetXRsvd__5 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__5"))
	return nil
}

// SetRxQueueDisabledDrops sets cunter in shared memory
func (mtr *LifMetrics) SetRxQueueDisabledDrops(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxQueueDisabledDrops"))
	return nil
}

// SetRxQueueEmptyDrops sets cunter in shared memory
func (mtr *LifMetrics) SetRxQueueEmptyDrops(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxQueueEmptyDrops"))
	return nil
}

// SetRxQueueSched sets cunter in shared memory
func (mtr *LifMetrics) SetRxQueueSched(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxQueueSched"))
	return nil
}

// SetRxDescFetchError sets cunter in shared memory
func (mtr *LifMetrics) SetRxDescFetchError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxDescFetchError"))
	return nil
}

// SetRxDescDataError sets cunter in shared memory
func (mtr *LifMetrics) SetRxDescDataError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxDescDataError"))
	return nil
}

// SetXRsvd__6 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__6"))
	return nil
}

// SetXRsvd__7 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__7"))
	return nil
}

// SetXRsvd__8 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__8"))
	return nil
}

// SetTxQueueDisabledDrops sets cunter in shared memory
func (mtr *LifMetrics) SetTxQueueDisabledDrops(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxQueueDisabledDrops"))
	return nil
}

// SetTxQueueSched sets cunter in shared memory
func (mtr *LifMetrics) SetTxQueueSched(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxQueueSched"))
	return nil
}

// SetTxDescFetchError sets cunter in shared memory
func (mtr *LifMetrics) SetTxDescFetchError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDescFetchError"))
	return nil
}

// SetTxDescDataError sets cunter in shared memory
func (mtr *LifMetrics) SetTxDescDataError(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDescDataError"))
	return nil
}

// SetXRsvd__9 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__9"))
	return nil
}

// SetXRsvd__10 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__10"))
	return nil
}

// SetXRsvd__11 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__11"))
	return nil
}

// SetXRsvd__12 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__12(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__12"))
	return nil
}

// SetRxRss sets cunter in shared memory
func (mtr *LifMetrics) SetRxRss(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxRss"))
	return nil
}

// SetRxCsumComplete sets cunter in shared memory
func (mtr *LifMetrics) SetRxCsumComplete(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxCsumComplete"))
	return nil
}

// SetRxIpBad sets cunter in shared memory
func (mtr *LifMetrics) SetRxIpBad(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxIpBad"))
	return nil
}

// SetRxTcpBad sets cunter in shared memory
func (mtr *LifMetrics) SetRxTcpBad(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxTcpBad"))
	return nil
}

// SetRxUdpBad sets cunter in shared memory
func (mtr *LifMetrics) SetRxUdpBad(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxUdpBad"))
	return nil
}

// SetRxVlanStrip sets cunter in shared memory
func (mtr *LifMetrics) SetRxVlanStrip(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxVlanStrip"))
	return nil
}

// SetXRsvd__13 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__13(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__13"))
	return nil
}

// SetXRsvd__14 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__14(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__14"))
	return nil
}

// SetTxCsumHw sets cunter in shared memory
func (mtr *LifMetrics) SetTxCsumHw(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxCsumHw"))
	return nil
}

// SetTxCsumHwInner sets cunter in shared memory
func (mtr *LifMetrics) SetTxCsumHwInner(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxCsumHwInner"))
	return nil
}

// SetTxVlanInsert sets cunter in shared memory
func (mtr *LifMetrics) SetTxVlanInsert(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxVlanInsert"))
	return nil
}

// SetTxSg sets cunter in shared memory
func (mtr *LifMetrics) SetTxSg(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxSg"))
	return nil
}

// SetTxTsoSg sets cunter in shared memory
func (mtr *LifMetrics) SetTxTsoSg(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxTsoSg"))
	return nil
}

// SetTxTsoSop sets cunter in shared memory
func (mtr *LifMetrics) SetTxTsoSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxTsoSop"))
	return nil
}

// SetTxTsoEop sets cunter in shared memory
func (mtr *LifMetrics) SetTxTsoEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxTsoEop"))
	return nil
}

// SetXRsvd__15 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__15(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__15"))
	return nil
}

// SetTxDescOpcInvalid sets cunter in shared memory
func (mtr *LifMetrics) SetTxDescOpcInvalid(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDescOpcInvalid"))
	return nil
}

// SetTxDescOpcCsumNone sets cunter in shared memory
func (mtr *LifMetrics) SetTxDescOpcCsumNone(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDescOpcCsumNone"))
	return nil
}

// SetTxDescOpcCsumPartial sets cunter in shared memory
func (mtr *LifMetrics) SetTxDescOpcCsumPartial(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDescOpcCsumPartial"))
	return nil
}

// SetTxDescOpcCsumHw sets cunter in shared memory
func (mtr *LifMetrics) SetTxDescOpcCsumHw(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDescOpcCsumHw"))
	return nil
}

// SetTxDescOpcCsumTso sets cunter in shared memory
func (mtr *LifMetrics) SetTxDescOpcCsumTso(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDescOpcCsumTso"))
	return nil
}

// SetXRsvd__16 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__16(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__16"))
	return nil
}

// SetXRsvd__17 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__17(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__17"))
	return nil
}

// SetXRsvd__18 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__18(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__18"))
	return nil
}

// LifMetricsIterator is the iterator object
type LifMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *LifMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *LifMetricsIterator) Next() *LifMetrics {
	mtr := it.iter.Next()
	tmtr := &LifMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *LifMetricsIterator) Find(key uint64) (*LifMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &LifMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *LifMetricsIterator) Create(key uint64) (*LifMetrics, error) {
	tmtr := &LifMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &LifMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *LifMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *LifMetricsIterator) Free() {
	it.iter.Free()
}

// NewLifMetricsIterator returns an iterator
func NewLifMetricsIterator() (*LifMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("LifMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &LifMetricsIterator{iter: iter}, nil
}
