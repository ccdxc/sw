// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/golang/protobuf/proto"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
)

type LifMetrics struct {
	key uint64

	RxUnicastPackets gometrics.Counter

	RxMulticastPackets gometrics.Counter

	RxBroadcastPackets gometrics.Counter

	XRsvd__0 gometrics.Counter

	RxUnicastBytes gometrics.Counter

	RxBroadcastBytes gometrics.Counter

	RxMulticastBytes gometrics.Counter

	XRsvd__1 gometrics.Counter

	RxQueueEmptyDrops gometrics.Counter

	RxQueueDisabledDrops gometrics.Counter

	RxDescError gometrics.Counter

	XRsvd__2 gometrics.Counter

	XRsvd__3 gometrics.Counter

	XRsvd__4 gometrics.Counter

	XRsvd__5 gometrics.Counter

	XRsvd__6 gometrics.Counter

	TxCsumNone gometrics.Counter

	TxCsumHw gometrics.Counter

	TxCsumHwInner gometrics.Counter

	TxCsumPartial gometrics.Counter

	TxSg gometrics.Counter

	TxTso gometrics.Counter

	TxTsoSop gometrics.Counter

	TxTsoEop gometrics.Counter

	TxOpError gometrics.Counter

	XRsvd__7 gometrics.Counter

	XRsvd__8 gometrics.Counter

	XRsvd__9 gometrics.Counter

	XRsvd__10 gometrics.Counter

	XRsvd__11 gometrics.Counter

	XRsvd__12 gometrics.Counter

	XRsvd__13 gometrics.Counter

	TxUnicastPackets gometrics.Counter

	TxUnicastBytes gometrics.Counter

	TxMulticastPackets gometrics.Counter

	TxMulticastBytes gometrics.Counter

	TxBroadcastPackets gometrics.Counter

	TxBroadcastBytes gometrics.Counter

	XRsvd__14 gometrics.Counter

	XRsvd__15 gometrics.Counter

	TxDropUnicastPackets gometrics.Counter

	TxDropUnicastBytes gometrics.Counter

	TxDropMulticastPackets gometrics.Counter

	TxDropMulticastBytes gometrics.Counter

	TxDropBroadcastPackets gometrics.Counter

	TxDropBroadcastBytes gometrics.Counter

	TxDmaError gometrics.Counter

	XRsvd__16 gometrics.Counter

	TxQueueDisabled gometrics.Counter

	TxQueueSched gometrics.Counter

	TxDescError gometrics.Counter

	XRsvd__17 gometrics.Counter

	XRsvd__18 gometrics.Counter

	XRsvd__19 gometrics.Counter

	XRsvd__20 gometrics.Counter

	XRsvd__21 gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *LifMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *LifMetrics) Size() int {
	sz := 0

	sz += mtr.RxUnicastPackets.Size()

	sz += mtr.RxMulticastPackets.Size()

	sz += mtr.RxBroadcastPackets.Size()

	sz += mtr.XRsvd__0.Size()

	sz += mtr.RxUnicastBytes.Size()

	sz += mtr.RxBroadcastBytes.Size()

	sz += mtr.RxMulticastBytes.Size()

	sz += mtr.XRsvd__1.Size()

	sz += mtr.RxQueueEmptyDrops.Size()

	sz += mtr.RxQueueDisabledDrops.Size()

	sz += mtr.RxDescError.Size()

	sz += mtr.XRsvd__2.Size()

	sz += mtr.XRsvd__3.Size()

	sz += mtr.XRsvd__4.Size()

	sz += mtr.XRsvd__5.Size()

	sz += mtr.XRsvd__6.Size()

	sz += mtr.TxCsumNone.Size()

	sz += mtr.TxCsumHw.Size()

	sz += mtr.TxCsumHwInner.Size()

	sz += mtr.TxCsumPartial.Size()

	sz += mtr.TxSg.Size()

	sz += mtr.TxTso.Size()

	sz += mtr.TxTsoSop.Size()

	sz += mtr.TxTsoEop.Size()

	sz += mtr.TxOpError.Size()

	sz += mtr.XRsvd__7.Size()

	sz += mtr.XRsvd__8.Size()

	sz += mtr.XRsvd__9.Size()

	sz += mtr.XRsvd__10.Size()

	sz += mtr.XRsvd__11.Size()

	sz += mtr.XRsvd__12.Size()

	sz += mtr.XRsvd__13.Size()

	sz += mtr.TxUnicastPackets.Size()

	sz += mtr.TxUnicastBytes.Size()

	sz += mtr.TxMulticastPackets.Size()

	sz += mtr.TxMulticastBytes.Size()

	sz += mtr.TxBroadcastPackets.Size()

	sz += mtr.TxBroadcastBytes.Size()

	sz += mtr.XRsvd__14.Size()

	sz += mtr.XRsvd__15.Size()

	sz += mtr.TxDropUnicastPackets.Size()

	sz += mtr.TxDropUnicastBytes.Size()

	sz += mtr.TxDropMulticastPackets.Size()

	sz += mtr.TxDropMulticastBytes.Size()

	sz += mtr.TxDropBroadcastPackets.Size()

	sz += mtr.TxDropBroadcastBytes.Size()

	sz += mtr.TxDmaError.Size()

	sz += mtr.XRsvd__16.Size()

	sz += mtr.TxQueueDisabled.Size()

	sz += mtr.TxQueueSched.Size()

	sz += mtr.TxDescError.Size()

	sz += mtr.XRsvd__17.Size()

	sz += mtr.XRsvd__18.Size()

	sz += mtr.XRsvd__19.Size()

	sz += mtr.XRsvd__20.Size()

	sz += mtr.XRsvd__21.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *LifMetrics) Unmarshal() error {
	var offset int

	val, _ := proto.DecodeVarint([]byte(mtr.metrics.GetKey()))
	mtr.key = uint64(val)

	mtr.RxUnicastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RxUnicastPackets.Size()

	mtr.RxMulticastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RxMulticastPackets.Size()

	mtr.RxBroadcastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RxBroadcastPackets.Size()

	mtr.XRsvd__0 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__0.Size()

	mtr.RxUnicastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.RxUnicastBytes.Size()

	mtr.RxBroadcastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.RxBroadcastBytes.Size()

	mtr.RxMulticastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.RxMulticastBytes.Size()

	mtr.XRsvd__1 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__1.Size()

	mtr.RxQueueEmptyDrops = mtr.metrics.GetCounter(offset)
	offset += mtr.RxQueueEmptyDrops.Size()

	mtr.RxQueueDisabledDrops = mtr.metrics.GetCounter(offset)
	offset += mtr.RxQueueDisabledDrops.Size()

	mtr.RxDescError = mtr.metrics.GetCounter(offset)
	offset += mtr.RxDescError.Size()

	mtr.XRsvd__2 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__2.Size()

	mtr.XRsvd__3 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__3.Size()

	mtr.XRsvd__4 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__4.Size()

	mtr.XRsvd__5 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__5.Size()

	mtr.XRsvd__6 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__6.Size()

	mtr.TxCsumNone = mtr.metrics.GetCounter(offset)
	offset += mtr.TxCsumNone.Size()

	mtr.TxCsumHw = mtr.metrics.GetCounter(offset)
	offset += mtr.TxCsumHw.Size()

	mtr.TxCsumHwInner = mtr.metrics.GetCounter(offset)
	offset += mtr.TxCsumHwInner.Size()

	mtr.TxCsumPartial = mtr.metrics.GetCounter(offset)
	offset += mtr.TxCsumPartial.Size()

	mtr.TxSg = mtr.metrics.GetCounter(offset)
	offset += mtr.TxSg.Size()

	mtr.TxTso = mtr.metrics.GetCounter(offset)
	offset += mtr.TxTso.Size()

	mtr.TxTsoSop = mtr.metrics.GetCounter(offset)
	offset += mtr.TxTsoSop.Size()

	mtr.TxTsoEop = mtr.metrics.GetCounter(offset)
	offset += mtr.TxTsoEop.Size()

	mtr.TxOpError = mtr.metrics.GetCounter(offset)
	offset += mtr.TxOpError.Size()

	mtr.XRsvd__7 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__7.Size()

	mtr.XRsvd__8 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__8.Size()

	mtr.XRsvd__9 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__9.Size()

	mtr.XRsvd__10 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__10.Size()

	mtr.XRsvd__11 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__11.Size()

	mtr.XRsvd__12 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__12.Size()

	mtr.XRsvd__13 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__13.Size()

	mtr.TxUnicastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxUnicastPackets.Size()

	mtr.TxUnicastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxUnicastBytes.Size()

	mtr.TxMulticastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxMulticastPackets.Size()

	mtr.TxMulticastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxMulticastBytes.Size()

	mtr.TxBroadcastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxBroadcastPackets.Size()

	mtr.TxBroadcastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxBroadcastBytes.Size()

	mtr.XRsvd__14 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__14.Size()

	mtr.XRsvd__15 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__15.Size()

	mtr.TxDropUnicastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDropUnicastPackets.Size()

	mtr.TxDropUnicastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDropUnicastBytes.Size()

	mtr.TxDropMulticastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDropMulticastPackets.Size()

	mtr.TxDropMulticastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDropMulticastBytes.Size()

	mtr.TxDropBroadcastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDropBroadcastPackets.Size()

	mtr.TxDropBroadcastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDropBroadcastBytes.Size()

	mtr.TxDmaError = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDmaError.Size()

	mtr.XRsvd__16 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__16.Size()

	mtr.TxQueueDisabled = mtr.metrics.GetCounter(offset)
	offset += mtr.TxQueueDisabled.Size()

	mtr.TxQueueSched = mtr.metrics.GetCounter(offset)
	offset += mtr.TxQueueSched.Size()

	mtr.TxDescError = mtr.metrics.GetCounter(offset)
	offset += mtr.TxDescError.Size()

	mtr.XRsvd__17 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__17.Size()

	mtr.XRsvd__18 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__18.Size()

	mtr.XRsvd__19 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__19.Size()

	mtr.XRsvd__20 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__20.Size()

	mtr.XRsvd__21 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__21.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *LifMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RxUnicastPackets" {
		return offset
	}
	offset += mtr.RxUnicastPackets.Size()

	if fldName == "RxMulticastPackets" {
		return offset
	}
	offset += mtr.RxMulticastPackets.Size()

	if fldName == "RxBroadcastPackets" {
		return offset
	}
	offset += mtr.RxBroadcastPackets.Size()

	if fldName == "XRsvd__0" {
		return offset
	}
	offset += mtr.XRsvd__0.Size()

	if fldName == "RxUnicastBytes" {
		return offset
	}
	offset += mtr.RxUnicastBytes.Size()

	if fldName == "RxBroadcastBytes" {
		return offset
	}
	offset += mtr.RxBroadcastBytes.Size()

	if fldName == "RxMulticastBytes" {
		return offset
	}
	offset += mtr.RxMulticastBytes.Size()

	if fldName == "XRsvd__1" {
		return offset
	}
	offset += mtr.XRsvd__1.Size()

	if fldName == "RxQueueEmptyDrops" {
		return offset
	}
	offset += mtr.RxQueueEmptyDrops.Size()

	if fldName == "RxQueueDisabledDrops" {
		return offset
	}
	offset += mtr.RxQueueDisabledDrops.Size()

	if fldName == "RxDescError" {
		return offset
	}
	offset += mtr.RxDescError.Size()

	if fldName == "XRsvd__2" {
		return offset
	}
	offset += mtr.XRsvd__2.Size()

	if fldName == "XRsvd__3" {
		return offset
	}
	offset += mtr.XRsvd__3.Size()

	if fldName == "XRsvd__4" {
		return offset
	}
	offset += mtr.XRsvd__4.Size()

	if fldName == "XRsvd__5" {
		return offset
	}
	offset += mtr.XRsvd__5.Size()

	if fldName == "XRsvd__6" {
		return offset
	}
	offset += mtr.XRsvd__6.Size()

	if fldName == "TxCsumNone" {
		return offset
	}
	offset += mtr.TxCsumNone.Size()

	if fldName == "TxCsumHw" {
		return offset
	}
	offset += mtr.TxCsumHw.Size()

	if fldName == "TxCsumHwInner" {
		return offset
	}
	offset += mtr.TxCsumHwInner.Size()

	if fldName == "TxCsumPartial" {
		return offset
	}
	offset += mtr.TxCsumPartial.Size()

	if fldName == "TxSg" {
		return offset
	}
	offset += mtr.TxSg.Size()

	if fldName == "TxTso" {
		return offset
	}
	offset += mtr.TxTso.Size()

	if fldName == "TxTsoSop" {
		return offset
	}
	offset += mtr.TxTsoSop.Size()

	if fldName == "TxTsoEop" {
		return offset
	}
	offset += mtr.TxTsoEop.Size()

	if fldName == "TxOpError" {
		return offset
	}
	offset += mtr.TxOpError.Size()

	if fldName == "XRsvd__7" {
		return offset
	}
	offset += mtr.XRsvd__7.Size()

	if fldName == "XRsvd__8" {
		return offset
	}
	offset += mtr.XRsvd__8.Size()

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

	if fldName == "XRsvd__13" {
		return offset
	}
	offset += mtr.XRsvd__13.Size()

	if fldName == "TxUnicastPackets" {
		return offset
	}
	offset += mtr.TxUnicastPackets.Size()

	if fldName == "TxUnicastBytes" {
		return offset
	}
	offset += mtr.TxUnicastBytes.Size()

	if fldName == "TxMulticastPackets" {
		return offset
	}
	offset += mtr.TxMulticastPackets.Size()

	if fldName == "TxMulticastBytes" {
		return offset
	}
	offset += mtr.TxMulticastBytes.Size()

	if fldName == "TxBroadcastPackets" {
		return offset
	}
	offset += mtr.TxBroadcastPackets.Size()

	if fldName == "TxBroadcastBytes" {
		return offset
	}
	offset += mtr.TxBroadcastBytes.Size()

	if fldName == "XRsvd__14" {
		return offset
	}
	offset += mtr.XRsvd__14.Size()

	if fldName == "XRsvd__15" {
		return offset
	}
	offset += mtr.XRsvd__15.Size()

	if fldName == "TxDropUnicastPackets" {
		return offset
	}
	offset += mtr.TxDropUnicastPackets.Size()

	if fldName == "TxDropUnicastBytes" {
		return offset
	}
	offset += mtr.TxDropUnicastBytes.Size()

	if fldName == "TxDropMulticastPackets" {
		return offset
	}
	offset += mtr.TxDropMulticastPackets.Size()

	if fldName == "TxDropMulticastBytes" {
		return offset
	}
	offset += mtr.TxDropMulticastBytes.Size()

	if fldName == "TxDropBroadcastPackets" {
		return offset
	}
	offset += mtr.TxDropBroadcastPackets.Size()

	if fldName == "TxDropBroadcastBytes" {
		return offset
	}
	offset += mtr.TxDropBroadcastBytes.Size()

	if fldName == "TxDmaError" {
		return offset
	}
	offset += mtr.TxDmaError.Size()

	if fldName == "XRsvd__16" {
		return offset
	}
	offset += mtr.XRsvd__16.Size()

	if fldName == "TxQueueDisabled" {
		return offset
	}
	offset += mtr.TxQueueDisabled.Size()

	if fldName == "TxQueueSched" {
		return offset
	}
	offset += mtr.TxQueueSched.Size()

	if fldName == "TxDescError" {
		return offset
	}
	offset += mtr.TxDescError.Size()

	if fldName == "XRsvd__17" {
		return offset
	}
	offset += mtr.XRsvd__17.Size()

	if fldName == "XRsvd__18" {
		return offset
	}
	offset += mtr.XRsvd__18.Size()

	if fldName == "XRsvd__19" {
		return offset
	}
	offset += mtr.XRsvd__19.Size()

	if fldName == "XRsvd__20" {
		return offset
	}
	offset += mtr.XRsvd__20.Size()

	if fldName == "XRsvd__21" {
		return offset
	}
	offset += mtr.XRsvd__21.Size()

	return offset
}

// SetRxUnicastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRxUnicastPackets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxUnicastPackets"))
	return nil
}

// SetRxMulticastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRxMulticastPackets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxMulticastPackets"))
	return nil
}

// SetRxBroadcastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRxBroadcastPackets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxBroadcastPackets"))
	return nil
}

// SetXRsvd__0 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__0"))
	return nil
}

// SetRxUnicastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetRxUnicastBytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxUnicastBytes"))
	return nil
}

// SetRxBroadcastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetRxBroadcastBytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxBroadcastBytes"))
	return nil
}

// SetRxMulticastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetRxMulticastBytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxMulticastBytes"))
	return nil
}

// SetXRsvd__1 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__1"))
	return nil
}

// SetRxQueueEmptyDrops sets cunter in shared memory
func (mtr *LifMetrics) SetRxQueueEmptyDrops(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxQueueEmptyDrops"))
	return nil
}

// SetRxQueueDisabledDrops sets cunter in shared memory
func (mtr *LifMetrics) SetRxQueueDisabledDrops(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxQueueDisabledDrops"))
	return nil
}

// SetRxDescError sets cunter in shared memory
func (mtr *LifMetrics) SetRxDescError(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxDescError"))
	return nil
}

// SetXRsvd__2 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__2"))
	return nil
}

// SetXRsvd__3 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__3"))
	return nil
}

// SetXRsvd__4 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__4"))
	return nil
}

// SetXRsvd__5 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__5(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__5"))
	return nil
}

// SetXRsvd__6 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__6(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__6"))
	return nil
}

// SetTxCsumNone sets cunter in shared memory
func (mtr *LifMetrics) SetTxCsumNone(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxCsumNone"))
	return nil
}

// SetTxCsumHw sets cunter in shared memory
func (mtr *LifMetrics) SetTxCsumHw(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxCsumHw"))
	return nil
}

// SetTxCsumHwInner sets cunter in shared memory
func (mtr *LifMetrics) SetTxCsumHwInner(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxCsumHwInner"))
	return nil
}

// SetTxCsumPartial sets cunter in shared memory
func (mtr *LifMetrics) SetTxCsumPartial(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxCsumPartial"))
	return nil
}

// SetTxSg sets cunter in shared memory
func (mtr *LifMetrics) SetTxSg(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxSg"))
	return nil
}

// SetTxTso sets cunter in shared memory
func (mtr *LifMetrics) SetTxTso(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxTso"))
	return nil
}

// SetTxTsoSop sets cunter in shared memory
func (mtr *LifMetrics) SetTxTsoSop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxTsoSop"))
	return nil
}

// SetTxTsoEop sets cunter in shared memory
func (mtr *LifMetrics) SetTxTsoEop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxTsoEop"))
	return nil
}

// SetTxOpError sets cunter in shared memory
func (mtr *LifMetrics) SetTxOpError(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxOpError"))
	return nil
}

// SetXRsvd__7 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__7(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__7"))
	return nil
}

// SetXRsvd__8 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__8(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__8"))
	return nil
}

// SetXRsvd__9 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__9(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__9"))
	return nil
}

// SetXRsvd__10 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__10(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__10"))
	return nil
}

// SetXRsvd__11 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__11(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__11"))
	return nil
}

// SetXRsvd__12 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__12(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__12"))
	return nil
}

// SetXRsvd__13 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__13(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__13"))
	return nil
}

// SetTxUnicastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxUnicastPackets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxUnicastPackets"))
	return nil
}

// SetTxUnicastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxUnicastBytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxUnicastBytes"))
	return nil
}

// SetTxMulticastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxMulticastPackets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxMulticastPackets"))
	return nil
}

// SetTxMulticastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxMulticastBytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxMulticastBytes"))
	return nil
}

// SetTxBroadcastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxBroadcastPackets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxBroadcastPackets"))
	return nil
}

// SetTxBroadcastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxBroadcastBytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxBroadcastBytes"))
	return nil
}

// SetXRsvd__14 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__14(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__14"))
	return nil
}

// SetXRsvd__15 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__15(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__15"))
	return nil
}

// SetTxDropUnicastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxDropUnicastPackets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDropUnicastPackets"))
	return nil
}

// SetTxDropUnicastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxDropUnicastBytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDropUnicastBytes"))
	return nil
}

// SetTxDropMulticastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxDropMulticastPackets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDropMulticastPackets"))
	return nil
}

// SetTxDropMulticastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxDropMulticastBytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDropMulticastBytes"))
	return nil
}

// SetTxDropBroadcastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxDropBroadcastPackets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDropBroadcastPackets"))
	return nil
}

// SetTxDropBroadcastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxDropBroadcastBytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDropBroadcastBytes"))
	return nil
}

// SetTxDmaError sets cunter in shared memory
func (mtr *LifMetrics) SetTxDmaError(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDmaError"))
	return nil
}

// SetXRsvd__16 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__16(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__16"))
	return nil
}

// SetTxQueueDisabled sets cunter in shared memory
func (mtr *LifMetrics) SetTxQueueDisabled(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxQueueDisabled"))
	return nil
}

// SetTxQueueSched sets cunter in shared memory
func (mtr *LifMetrics) SetTxQueueSched(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxQueueSched"))
	return nil
}

// SetTxDescError sets cunter in shared memory
func (mtr *LifMetrics) SetTxDescError(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxDescError"))
	return nil
}

// SetXRsvd__17 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__17(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__17"))
	return nil
}

// SetXRsvd__18 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__18(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__18"))
	return nil
}

// SetXRsvd__19 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__19(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__19"))
	return nil
}

// SetXRsvd__20 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__20(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__20"))
	return nil
}

// SetXRsvd__21 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__21(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__21"))
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

	mtr, err := it.iter.Find(string(proto.EncodeVarint(uint64(key))))

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

	mtr := it.iter.Create(string(proto.EncodeVarint(uint64(key))), tmtr.Size())

	tmtr = &LifMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *LifMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(string(proto.EncodeVarint(uint64(key))))

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
