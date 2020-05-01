// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/tsdb/metrics"
)

type MacMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint32

	FramesRxOk metrics.Counter

	FramesRxAll metrics.Counter

	FramesRxBadFcs metrics.Counter

	FramesRxBadAll metrics.Counter

	OctetsRxOk metrics.Counter

	OctetsRxAll metrics.Counter

	FramesRxUnicast metrics.Counter

	FramesRxMulticast metrics.Counter

	FramesRxBroadcast metrics.Counter

	FramesRxPause metrics.Counter

	FramesRxBadLength metrics.Counter

	FramesRxUndersized metrics.Counter

	FramesRxOversized metrics.Counter

	FramesRxFragments metrics.Counter

	FramesRxJabber metrics.Counter

	FramesRxPripause metrics.Counter

	FramesRxStompedCrc metrics.Counter

	FramesRxTooLong metrics.Counter

	FramesRxVlanGood metrics.Counter

	FramesRxDropped metrics.Counter

	FramesRxLessThan_64B metrics.Counter

	FramesRx_64B metrics.Counter

	FramesRx_65B_127B metrics.Counter

	FramesRx_128B_255B metrics.Counter

	FramesRx_256B_511B metrics.Counter

	FramesRx_512B_1023B metrics.Counter

	FramesRx_1024B_1518B metrics.Counter

	FramesRx_1519B_2047B metrics.Counter

	FramesRx_2048B_4095B metrics.Counter

	FramesRx_4096B_8191B metrics.Counter

	FramesRx_8192B_9215B metrics.Counter

	FramesRxOther metrics.Counter

	FramesTxOk metrics.Counter

	FramesTxAll metrics.Counter

	FramesTxBad metrics.Counter

	OctetsTxOk metrics.Counter

	OctetsTxTotal metrics.Counter

	FramesTxUnicast metrics.Counter

	FramesTxMulticast metrics.Counter

	FramesTxBroadcast metrics.Counter

	FramesTxPause metrics.Counter

	FramesTxPripause metrics.Counter

	FramesTxVlan metrics.Counter

	FramesTxLessThan_64B metrics.Counter

	FramesTx_64B metrics.Counter

	FramesTx_65B_127B metrics.Counter

	FramesTx_128B_255B metrics.Counter

	FramesTx_256B_511B metrics.Counter

	FramesTx_512B_1023B metrics.Counter

	FramesTx_1024B_1518B metrics.Counter

	FramesTx_1519B_2047B metrics.Counter

	FramesTx_2048B_4095B metrics.Counter

	FramesTx_4096B_8191B metrics.Counter

	FramesTx_8192B_9215B metrics.Counter

	FramesTxOther metrics.Counter

	FramesTxPri_0 metrics.Counter

	FramesTxPri_1 metrics.Counter

	FramesTxPri_2 metrics.Counter

	FramesTxPri_3 metrics.Counter

	FramesTxPri_4 metrics.Counter

	FramesTxPri_5 metrics.Counter

	FramesTxPri_6 metrics.Counter

	FramesTxPri_7 metrics.Counter

	FramesRxPri_0 metrics.Counter

	FramesRxPri_1 metrics.Counter

	FramesRxPri_2 metrics.Counter

	FramesRxPri_3 metrics.Counter

	FramesRxPri_4 metrics.Counter

	FramesRxPri_5 metrics.Counter

	FramesRxPri_6 metrics.Counter

	FramesRxPri_7 metrics.Counter

	TxPripause_0_1UsCount metrics.Counter

	TxPripause_1_1UsCount metrics.Counter

	TxPripause_2_1UsCount metrics.Counter

	TxPripause_3_1UsCount metrics.Counter

	TxPripause_4_1UsCount metrics.Counter

	TxPripause_5_1UsCount metrics.Counter

	TxPripause_6_1UsCount metrics.Counter

	TxPripause_7_1UsCount metrics.Counter

	RxPripause_0_1UsCount metrics.Counter

	RxPripause_1_1UsCount metrics.Counter

	RxPripause_2_1UsCount metrics.Counter

	RxPripause_3_1UsCount metrics.Counter

	RxPripause_4_1UsCount metrics.Counter

	RxPripause_5_1UsCount metrics.Counter

	RxPripause_6_1UsCount metrics.Counter

	RxPripause_7_1UsCount metrics.Counter

	RxPause_1UsCount metrics.Counter

	FramesTxTruncated metrics.Counter

	TxPps metrics.Counter

	TxBytesps metrics.Counter

	RxPps metrics.Counter

	RxBytesps metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *MacMetrics) GetKey() uint32 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *MacMetrics) Size() int {
	sz := 0

	sz += mtr.FramesRxOk.Size()

	sz += mtr.FramesRxAll.Size()

	sz += mtr.FramesRxBadFcs.Size()

	sz += mtr.FramesRxBadAll.Size()

	sz += mtr.OctetsRxOk.Size()

	sz += mtr.OctetsRxAll.Size()

	sz += mtr.FramesRxUnicast.Size()

	sz += mtr.FramesRxMulticast.Size()

	sz += mtr.FramesRxBroadcast.Size()

	sz += mtr.FramesRxPause.Size()

	sz += mtr.FramesRxBadLength.Size()

	sz += mtr.FramesRxUndersized.Size()

	sz += mtr.FramesRxOversized.Size()

	sz += mtr.FramesRxFragments.Size()

	sz += mtr.FramesRxJabber.Size()

	sz += mtr.FramesRxPripause.Size()

	sz += mtr.FramesRxStompedCrc.Size()

	sz += mtr.FramesRxTooLong.Size()

	sz += mtr.FramesRxVlanGood.Size()

	sz += mtr.FramesRxDropped.Size()

	sz += mtr.FramesRxLessThan_64B.Size()

	sz += mtr.FramesRx_64B.Size()

	sz += mtr.FramesRx_65B_127B.Size()

	sz += mtr.FramesRx_128B_255B.Size()

	sz += mtr.FramesRx_256B_511B.Size()

	sz += mtr.FramesRx_512B_1023B.Size()

	sz += mtr.FramesRx_1024B_1518B.Size()

	sz += mtr.FramesRx_1519B_2047B.Size()

	sz += mtr.FramesRx_2048B_4095B.Size()

	sz += mtr.FramesRx_4096B_8191B.Size()

	sz += mtr.FramesRx_8192B_9215B.Size()

	sz += mtr.FramesRxOther.Size()

	sz += mtr.FramesTxOk.Size()

	sz += mtr.FramesTxAll.Size()

	sz += mtr.FramesTxBad.Size()

	sz += mtr.OctetsTxOk.Size()

	sz += mtr.OctetsTxTotal.Size()

	sz += mtr.FramesTxUnicast.Size()

	sz += mtr.FramesTxMulticast.Size()

	sz += mtr.FramesTxBroadcast.Size()

	sz += mtr.FramesTxPause.Size()

	sz += mtr.FramesTxPripause.Size()

	sz += mtr.FramesTxVlan.Size()

	sz += mtr.FramesTxLessThan_64B.Size()

	sz += mtr.FramesTx_64B.Size()

	sz += mtr.FramesTx_65B_127B.Size()

	sz += mtr.FramesTx_128B_255B.Size()

	sz += mtr.FramesTx_256B_511B.Size()

	sz += mtr.FramesTx_512B_1023B.Size()

	sz += mtr.FramesTx_1024B_1518B.Size()

	sz += mtr.FramesTx_1519B_2047B.Size()

	sz += mtr.FramesTx_2048B_4095B.Size()

	sz += mtr.FramesTx_4096B_8191B.Size()

	sz += mtr.FramesTx_8192B_9215B.Size()

	sz += mtr.FramesTxOther.Size()

	sz += mtr.FramesTxPri_0.Size()

	sz += mtr.FramesTxPri_1.Size()

	sz += mtr.FramesTxPri_2.Size()

	sz += mtr.FramesTxPri_3.Size()

	sz += mtr.FramesTxPri_4.Size()

	sz += mtr.FramesTxPri_5.Size()

	sz += mtr.FramesTxPri_6.Size()

	sz += mtr.FramesTxPri_7.Size()

	sz += mtr.FramesRxPri_0.Size()

	sz += mtr.FramesRxPri_1.Size()

	sz += mtr.FramesRxPri_2.Size()

	sz += mtr.FramesRxPri_3.Size()

	sz += mtr.FramesRxPri_4.Size()

	sz += mtr.FramesRxPri_5.Size()

	sz += mtr.FramesRxPri_6.Size()

	sz += mtr.FramesRxPri_7.Size()

	sz += mtr.TxPripause_0_1UsCount.Size()

	sz += mtr.TxPripause_1_1UsCount.Size()

	sz += mtr.TxPripause_2_1UsCount.Size()

	sz += mtr.TxPripause_3_1UsCount.Size()

	sz += mtr.TxPripause_4_1UsCount.Size()

	sz += mtr.TxPripause_5_1UsCount.Size()

	sz += mtr.TxPripause_6_1UsCount.Size()

	sz += mtr.TxPripause_7_1UsCount.Size()

	sz += mtr.RxPripause_0_1UsCount.Size()

	sz += mtr.RxPripause_1_1UsCount.Size()

	sz += mtr.RxPripause_2_1UsCount.Size()

	sz += mtr.RxPripause_3_1UsCount.Size()

	sz += mtr.RxPripause_4_1UsCount.Size()

	sz += mtr.RxPripause_5_1UsCount.Size()

	sz += mtr.RxPripause_6_1UsCount.Size()

	sz += mtr.RxPripause_7_1UsCount.Size()

	sz += mtr.RxPause_1UsCount.Size()

	sz += mtr.FramesTxTruncated.Size()

	sz += mtr.TxPps.Size()

	sz += mtr.TxBytesps.Size()

	sz += mtr.RxPps.Size()

	sz += mtr.RxBytesps.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *MacMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.FramesRxOk = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxOk.Size()

	mtr.FramesRxAll = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxAll.Size()

	mtr.FramesRxBadFcs = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxBadFcs.Size()

	mtr.FramesRxBadAll = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxBadAll.Size()

	mtr.OctetsRxOk = mtr.metrics.GetCounter(offset)
	offset += mtr.OctetsRxOk.Size()

	mtr.OctetsRxAll = mtr.metrics.GetCounter(offset)
	offset += mtr.OctetsRxAll.Size()

	mtr.FramesRxUnicast = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxUnicast.Size()

	mtr.FramesRxMulticast = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxMulticast.Size()

	mtr.FramesRxBroadcast = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxBroadcast.Size()

	mtr.FramesRxPause = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxPause.Size()

	mtr.FramesRxBadLength = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxBadLength.Size()

	mtr.FramesRxUndersized = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxUndersized.Size()

	mtr.FramesRxOversized = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxOversized.Size()

	mtr.FramesRxFragments = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxFragments.Size()

	mtr.FramesRxJabber = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxJabber.Size()

	mtr.FramesRxPripause = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxPripause.Size()

	mtr.FramesRxStompedCrc = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxStompedCrc.Size()

	mtr.FramesRxTooLong = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxTooLong.Size()

	mtr.FramesRxVlanGood = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxVlanGood.Size()

	mtr.FramesRxDropped = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxDropped.Size()

	mtr.FramesRxLessThan_64B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxLessThan_64B.Size()

	mtr.FramesRx_64B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_64B.Size()

	mtr.FramesRx_65B_127B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_65B_127B.Size()

	mtr.FramesRx_128B_255B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_128B_255B.Size()

	mtr.FramesRx_256B_511B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_256B_511B.Size()

	mtr.FramesRx_512B_1023B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_512B_1023B.Size()

	mtr.FramesRx_1024B_1518B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_1024B_1518B.Size()

	mtr.FramesRx_1519B_2047B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_1519B_2047B.Size()

	mtr.FramesRx_2048B_4095B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_2048B_4095B.Size()

	mtr.FramesRx_4096B_8191B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_4096B_8191B.Size()

	mtr.FramesRx_8192B_9215B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_8192B_9215B.Size()

	mtr.FramesRxOther = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxOther.Size()

	mtr.FramesTxOk = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxOk.Size()

	mtr.FramesTxAll = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxAll.Size()

	mtr.FramesTxBad = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxBad.Size()

	mtr.OctetsTxOk = mtr.metrics.GetCounter(offset)
	offset += mtr.OctetsTxOk.Size()

	mtr.OctetsTxTotal = mtr.metrics.GetCounter(offset)
	offset += mtr.OctetsTxTotal.Size()

	mtr.FramesTxUnicast = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxUnicast.Size()

	mtr.FramesTxMulticast = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxMulticast.Size()

	mtr.FramesTxBroadcast = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxBroadcast.Size()

	mtr.FramesTxPause = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxPause.Size()

	mtr.FramesTxPripause = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxPripause.Size()

	mtr.FramesTxVlan = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxVlan.Size()

	mtr.FramesTxLessThan_64B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxLessThan_64B.Size()

	mtr.FramesTx_64B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTx_64B.Size()

	mtr.FramesTx_65B_127B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTx_65B_127B.Size()

	mtr.FramesTx_128B_255B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTx_128B_255B.Size()

	mtr.FramesTx_256B_511B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTx_256B_511B.Size()

	mtr.FramesTx_512B_1023B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTx_512B_1023B.Size()

	mtr.FramesTx_1024B_1518B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTx_1024B_1518B.Size()

	mtr.FramesTx_1519B_2047B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTx_1519B_2047B.Size()

	mtr.FramesTx_2048B_4095B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTx_2048B_4095B.Size()

	mtr.FramesTx_4096B_8191B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTx_4096B_8191B.Size()

	mtr.FramesTx_8192B_9215B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTx_8192B_9215B.Size()

	mtr.FramesTxOther = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxOther.Size()

	mtr.FramesTxPri_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxPri_0.Size()

	mtr.FramesTxPri_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxPri_1.Size()

	mtr.FramesTxPri_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxPri_2.Size()

	mtr.FramesTxPri_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxPri_3.Size()

	mtr.FramesTxPri_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxPri_4.Size()

	mtr.FramesTxPri_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxPri_5.Size()

	mtr.FramesTxPri_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxPri_6.Size()

	mtr.FramesTxPri_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxPri_7.Size()

	mtr.FramesRxPri_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxPri_0.Size()

	mtr.FramesRxPri_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxPri_1.Size()

	mtr.FramesRxPri_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxPri_2.Size()

	mtr.FramesRxPri_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxPri_3.Size()

	mtr.FramesRxPri_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxPri_4.Size()

	mtr.FramesRxPri_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxPri_5.Size()

	mtr.FramesRxPri_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxPri_6.Size()

	mtr.FramesRxPri_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxPri_7.Size()

	mtr.TxPripause_0_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPripause_0_1UsCount.Size()

	mtr.TxPripause_1_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPripause_1_1UsCount.Size()

	mtr.TxPripause_2_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPripause_2_1UsCount.Size()

	mtr.TxPripause_3_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPripause_3_1UsCount.Size()

	mtr.TxPripause_4_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPripause_4_1UsCount.Size()

	mtr.TxPripause_5_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPripause_5_1UsCount.Size()

	mtr.TxPripause_6_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPripause_6_1UsCount.Size()

	mtr.TxPripause_7_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPripause_7_1UsCount.Size()

	mtr.RxPripause_0_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPripause_0_1UsCount.Size()

	mtr.RxPripause_1_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPripause_1_1UsCount.Size()

	mtr.RxPripause_2_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPripause_2_1UsCount.Size()

	mtr.RxPripause_3_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPripause_3_1UsCount.Size()

	mtr.RxPripause_4_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPripause_4_1UsCount.Size()

	mtr.RxPripause_5_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPripause_5_1UsCount.Size()

	mtr.RxPripause_6_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPripause_6_1UsCount.Size()

	mtr.RxPripause_7_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPripause_7_1UsCount.Size()

	mtr.RxPause_1UsCount = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPause_1UsCount.Size()

	mtr.FramesTxTruncated = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxTruncated.Size()

	mtr.TxPps = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPps.Size()

	mtr.TxBytesps = mtr.metrics.GetCounter(offset)
	offset += mtr.TxBytesps.Size()

	mtr.RxPps = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPps.Size()

	mtr.RxBytesps = mtr.metrics.GetCounter(offset)
	offset += mtr.RxBytesps.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *MacMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "FramesRxOk" {
		return offset
	}
	offset += mtr.FramesRxOk.Size()

	if fldName == "FramesRxAll" {
		return offset
	}
	offset += mtr.FramesRxAll.Size()

	if fldName == "FramesRxBadFcs" {
		return offset
	}
	offset += mtr.FramesRxBadFcs.Size()

	if fldName == "FramesRxBadAll" {
		return offset
	}
	offset += mtr.FramesRxBadAll.Size()

	if fldName == "OctetsRxOk" {
		return offset
	}
	offset += mtr.OctetsRxOk.Size()

	if fldName == "OctetsRxAll" {
		return offset
	}
	offset += mtr.OctetsRxAll.Size()

	if fldName == "FramesRxUnicast" {
		return offset
	}
	offset += mtr.FramesRxUnicast.Size()

	if fldName == "FramesRxMulticast" {
		return offset
	}
	offset += mtr.FramesRxMulticast.Size()

	if fldName == "FramesRxBroadcast" {
		return offset
	}
	offset += mtr.FramesRxBroadcast.Size()

	if fldName == "FramesRxPause" {
		return offset
	}
	offset += mtr.FramesRxPause.Size()

	if fldName == "FramesRxBadLength" {
		return offset
	}
	offset += mtr.FramesRxBadLength.Size()

	if fldName == "FramesRxUndersized" {
		return offset
	}
	offset += mtr.FramesRxUndersized.Size()

	if fldName == "FramesRxOversized" {
		return offset
	}
	offset += mtr.FramesRxOversized.Size()

	if fldName == "FramesRxFragments" {
		return offset
	}
	offset += mtr.FramesRxFragments.Size()

	if fldName == "FramesRxJabber" {
		return offset
	}
	offset += mtr.FramesRxJabber.Size()

	if fldName == "FramesRxPripause" {
		return offset
	}
	offset += mtr.FramesRxPripause.Size()

	if fldName == "FramesRxStompedCrc" {
		return offset
	}
	offset += mtr.FramesRxStompedCrc.Size()

	if fldName == "FramesRxTooLong" {
		return offset
	}
	offset += mtr.FramesRxTooLong.Size()

	if fldName == "FramesRxVlanGood" {
		return offset
	}
	offset += mtr.FramesRxVlanGood.Size()

	if fldName == "FramesRxDropped" {
		return offset
	}
	offset += mtr.FramesRxDropped.Size()

	if fldName == "FramesRxLessThan_64B" {
		return offset
	}
	offset += mtr.FramesRxLessThan_64B.Size()

	if fldName == "FramesRx_64B" {
		return offset
	}
	offset += mtr.FramesRx_64B.Size()

	if fldName == "FramesRx_65B_127B" {
		return offset
	}
	offset += mtr.FramesRx_65B_127B.Size()

	if fldName == "FramesRx_128B_255B" {
		return offset
	}
	offset += mtr.FramesRx_128B_255B.Size()

	if fldName == "FramesRx_256B_511B" {
		return offset
	}
	offset += mtr.FramesRx_256B_511B.Size()

	if fldName == "FramesRx_512B_1023B" {
		return offset
	}
	offset += mtr.FramesRx_512B_1023B.Size()

	if fldName == "FramesRx_1024B_1518B" {
		return offset
	}
	offset += mtr.FramesRx_1024B_1518B.Size()

	if fldName == "FramesRx_1519B_2047B" {
		return offset
	}
	offset += mtr.FramesRx_1519B_2047B.Size()

	if fldName == "FramesRx_2048B_4095B" {
		return offset
	}
	offset += mtr.FramesRx_2048B_4095B.Size()

	if fldName == "FramesRx_4096B_8191B" {
		return offset
	}
	offset += mtr.FramesRx_4096B_8191B.Size()

	if fldName == "FramesRx_8192B_9215B" {
		return offset
	}
	offset += mtr.FramesRx_8192B_9215B.Size()

	if fldName == "FramesRxOther" {
		return offset
	}
	offset += mtr.FramesRxOther.Size()

	if fldName == "FramesTxOk" {
		return offset
	}
	offset += mtr.FramesTxOk.Size()

	if fldName == "FramesTxAll" {
		return offset
	}
	offset += mtr.FramesTxAll.Size()

	if fldName == "FramesTxBad" {
		return offset
	}
	offset += mtr.FramesTxBad.Size()

	if fldName == "OctetsTxOk" {
		return offset
	}
	offset += mtr.OctetsTxOk.Size()

	if fldName == "OctetsTxTotal" {
		return offset
	}
	offset += mtr.OctetsTxTotal.Size()

	if fldName == "FramesTxUnicast" {
		return offset
	}
	offset += mtr.FramesTxUnicast.Size()

	if fldName == "FramesTxMulticast" {
		return offset
	}
	offset += mtr.FramesTxMulticast.Size()

	if fldName == "FramesTxBroadcast" {
		return offset
	}
	offset += mtr.FramesTxBroadcast.Size()

	if fldName == "FramesTxPause" {
		return offset
	}
	offset += mtr.FramesTxPause.Size()

	if fldName == "FramesTxPripause" {
		return offset
	}
	offset += mtr.FramesTxPripause.Size()

	if fldName == "FramesTxVlan" {
		return offset
	}
	offset += mtr.FramesTxVlan.Size()

	if fldName == "FramesTxLessThan_64B" {
		return offset
	}
	offset += mtr.FramesTxLessThan_64B.Size()

	if fldName == "FramesTx_64B" {
		return offset
	}
	offset += mtr.FramesTx_64B.Size()

	if fldName == "FramesTx_65B_127B" {
		return offset
	}
	offset += mtr.FramesTx_65B_127B.Size()

	if fldName == "FramesTx_128B_255B" {
		return offset
	}
	offset += mtr.FramesTx_128B_255B.Size()

	if fldName == "FramesTx_256B_511B" {
		return offset
	}
	offset += mtr.FramesTx_256B_511B.Size()

	if fldName == "FramesTx_512B_1023B" {
		return offset
	}
	offset += mtr.FramesTx_512B_1023B.Size()

	if fldName == "FramesTx_1024B_1518B" {
		return offset
	}
	offset += mtr.FramesTx_1024B_1518B.Size()

	if fldName == "FramesTx_1519B_2047B" {
		return offset
	}
	offset += mtr.FramesTx_1519B_2047B.Size()

	if fldName == "FramesTx_2048B_4095B" {
		return offset
	}
	offset += mtr.FramesTx_2048B_4095B.Size()

	if fldName == "FramesTx_4096B_8191B" {
		return offset
	}
	offset += mtr.FramesTx_4096B_8191B.Size()

	if fldName == "FramesTx_8192B_9215B" {
		return offset
	}
	offset += mtr.FramesTx_8192B_9215B.Size()

	if fldName == "FramesTxOther" {
		return offset
	}
	offset += mtr.FramesTxOther.Size()

	if fldName == "FramesTxPri_0" {
		return offset
	}
	offset += mtr.FramesTxPri_0.Size()

	if fldName == "FramesTxPri_1" {
		return offset
	}
	offset += mtr.FramesTxPri_1.Size()

	if fldName == "FramesTxPri_2" {
		return offset
	}
	offset += mtr.FramesTxPri_2.Size()

	if fldName == "FramesTxPri_3" {
		return offset
	}
	offset += mtr.FramesTxPri_3.Size()

	if fldName == "FramesTxPri_4" {
		return offset
	}
	offset += mtr.FramesTxPri_4.Size()

	if fldName == "FramesTxPri_5" {
		return offset
	}
	offset += mtr.FramesTxPri_5.Size()

	if fldName == "FramesTxPri_6" {
		return offset
	}
	offset += mtr.FramesTxPri_6.Size()

	if fldName == "FramesTxPri_7" {
		return offset
	}
	offset += mtr.FramesTxPri_7.Size()

	if fldName == "FramesRxPri_0" {
		return offset
	}
	offset += mtr.FramesRxPri_0.Size()

	if fldName == "FramesRxPri_1" {
		return offset
	}
	offset += mtr.FramesRxPri_1.Size()

	if fldName == "FramesRxPri_2" {
		return offset
	}
	offset += mtr.FramesRxPri_2.Size()

	if fldName == "FramesRxPri_3" {
		return offset
	}
	offset += mtr.FramesRxPri_3.Size()

	if fldName == "FramesRxPri_4" {
		return offset
	}
	offset += mtr.FramesRxPri_4.Size()

	if fldName == "FramesRxPri_5" {
		return offset
	}
	offset += mtr.FramesRxPri_5.Size()

	if fldName == "FramesRxPri_6" {
		return offset
	}
	offset += mtr.FramesRxPri_6.Size()

	if fldName == "FramesRxPri_7" {
		return offset
	}
	offset += mtr.FramesRxPri_7.Size()

	if fldName == "TxPripause_0_1UsCount" {
		return offset
	}
	offset += mtr.TxPripause_0_1UsCount.Size()

	if fldName == "TxPripause_1_1UsCount" {
		return offset
	}
	offset += mtr.TxPripause_1_1UsCount.Size()

	if fldName == "TxPripause_2_1UsCount" {
		return offset
	}
	offset += mtr.TxPripause_2_1UsCount.Size()

	if fldName == "TxPripause_3_1UsCount" {
		return offset
	}
	offset += mtr.TxPripause_3_1UsCount.Size()

	if fldName == "TxPripause_4_1UsCount" {
		return offset
	}
	offset += mtr.TxPripause_4_1UsCount.Size()

	if fldName == "TxPripause_5_1UsCount" {
		return offset
	}
	offset += mtr.TxPripause_5_1UsCount.Size()

	if fldName == "TxPripause_6_1UsCount" {
		return offset
	}
	offset += mtr.TxPripause_6_1UsCount.Size()

	if fldName == "TxPripause_7_1UsCount" {
		return offset
	}
	offset += mtr.TxPripause_7_1UsCount.Size()

	if fldName == "RxPripause_0_1UsCount" {
		return offset
	}
	offset += mtr.RxPripause_0_1UsCount.Size()

	if fldName == "RxPripause_1_1UsCount" {
		return offset
	}
	offset += mtr.RxPripause_1_1UsCount.Size()

	if fldName == "RxPripause_2_1UsCount" {
		return offset
	}
	offset += mtr.RxPripause_2_1UsCount.Size()

	if fldName == "RxPripause_3_1UsCount" {
		return offset
	}
	offset += mtr.RxPripause_3_1UsCount.Size()

	if fldName == "RxPripause_4_1UsCount" {
		return offset
	}
	offset += mtr.RxPripause_4_1UsCount.Size()

	if fldName == "RxPripause_5_1UsCount" {
		return offset
	}
	offset += mtr.RxPripause_5_1UsCount.Size()

	if fldName == "RxPripause_6_1UsCount" {
		return offset
	}
	offset += mtr.RxPripause_6_1UsCount.Size()

	if fldName == "RxPripause_7_1UsCount" {
		return offset
	}
	offset += mtr.RxPripause_7_1UsCount.Size()

	if fldName == "RxPause_1UsCount" {
		return offset
	}
	offset += mtr.RxPause_1UsCount.Size()

	if fldName == "FramesTxTruncated" {
		return offset
	}
	offset += mtr.FramesTxTruncated.Size()

	if fldName == "TxPps" {
		return offset
	}
	offset += mtr.TxPps.Size()

	if fldName == "TxBytesps" {
		return offset
	}
	offset += mtr.TxBytesps.Size()

	if fldName == "RxPps" {
		return offset
	}
	offset += mtr.RxPps.Size()

	if fldName == "RxBytesps" {
		return offset
	}
	offset += mtr.RxBytesps.Size()

	return offset
}

// SetFramesRxOk sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxOk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxOk"))
	return nil
}

// SetFramesRxAll sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxAll(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxAll"))
	return nil
}

// SetFramesRxBadFcs sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxBadFcs(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxBadFcs"))
	return nil
}

// SetFramesRxBadAll sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxBadAll(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxBadAll"))
	return nil
}

// SetOctetsRxOk sets cunter in shared memory
func (mtr *MacMetrics) SetOctetsRxOk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OctetsRxOk"))
	return nil
}

// SetOctetsRxAll sets cunter in shared memory
func (mtr *MacMetrics) SetOctetsRxAll(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OctetsRxAll"))
	return nil
}

// SetFramesRxUnicast sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxUnicast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxUnicast"))
	return nil
}

// SetFramesRxMulticast sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxMulticast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxMulticast"))
	return nil
}

// SetFramesRxBroadcast sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxBroadcast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxBroadcast"))
	return nil
}

// SetFramesRxPause sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxPause(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxPause"))
	return nil
}

// SetFramesRxBadLength sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxBadLength(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxBadLength"))
	return nil
}

// SetFramesRxUndersized sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxUndersized(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxUndersized"))
	return nil
}

// SetFramesRxOversized sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxOversized(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxOversized"))
	return nil
}

// SetFramesRxFragments sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxFragments(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxFragments"))
	return nil
}

// SetFramesRxJabber sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxJabber(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxJabber"))
	return nil
}

// SetFramesRxPripause sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxPripause(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxPripause"))
	return nil
}

// SetFramesRxStompedCrc sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxStompedCrc(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxStompedCrc"))
	return nil
}

// SetFramesRxTooLong sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxTooLong(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxTooLong"))
	return nil
}

// SetFramesRxVlanGood sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxVlanGood(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxVlanGood"))
	return nil
}

// SetFramesRxDropped sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxDropped(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxDropped"))
	return nil
}

// SetFramesRxLessThan_64B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxLessThan_64B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxLessThan_64B"))
	return nil
}

// SetFramesRx_64B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRx_64B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_64B"))
	return nil
}

// SetFramesRx_65B_127B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRx_65B_127B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_65B_127B"))
	return nil
}

// SetFramesRx_128B_255B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRx_128B_255B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_128B_255B"))
	return nil
}

// SetFramesRx_256B_511B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRx_256B_511B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_256B_511B"))
	return nil
}

// SetFramesRx_512B_1023B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRx_512B_1023B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_512B_1023B"))
	return nil
}

// SetFramesRx_1024B_1518B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRx_1024B_1518B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_1024B_1518B"))
	return nil
}

// SetFramesRx_1519B_2047B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRx_1519B_2047B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_1519B_2047B"))
	return nil
}

// SetFramesRx_2048B_4095B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRx_2048B_4095B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_2048B_4095B"))
	return nil
}

// SetFramesRx_4096B_8191B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRx_4096B_8191B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_4096B_8191B"))
	return nil
}

// SetFramesRx_8192B_9215B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRx_8192B_9215B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_8192B_9215B"))
	return nil
}

// SetFramesRxOther sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxOther(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxOther"))
	return nil
}

// SetFramesTxOk sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxOk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxOk"))
	return nil
}

// SetFramesTxAll sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxAll(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxAll"))
	return nil
}

// SetFramesTxBad sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxBad(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxBad"))
	return nil
}

// SetOctetsTxOk sets cunter in shared memory
func (mtr *MacMetrics) SetOctetsTxOk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OctetsTxOk"))
	return nil
}

// SetOctetsTxTotal sets cunter in shared memory
func (mtr *MacMetrics) SetOctetsTxTotal(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OctetsTxTotal"))
	return nil
}

// SetFramesTxUnicast sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxUnicast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxUnicast"))
	return nil
}

// SetFramesTxMulticast sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxMulticast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxMulticast"))
	return nil
}

// SetFramesTxBroadcast sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxBroadcast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxBroadcast"))
	return nil
}

// SetFramesTxPause sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxPause(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxPause"))
	return nil
}

// SetFramesTxPripause sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxPripause(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxPripause"))
	return nil
}

// SetFramesTxVlan sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxVlan(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxVlan"))
	return nil
}

// SetFramesTxLessThan_64B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxLessThan_64B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxLessThan_64B"))
	return nil
}

// SetFramesTx_64B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTx_64B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTx_64B"))
	return nil
}

// SetFramesTx_65B_127B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTx_65B_127B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTx_65B_127B"))
	return nil
}

// SetFramesTx_128B_255B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTx_128B_255B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTx_128B_255B"))
	return nil
}

// SetFramesTx_256B_511B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTx_256B_511B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTx_256B_511B"))
	return nil
}

// SetFramesTx_512B_1023B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTx_512B_1023B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTx_512B_1023B"))
	return nil
}

// SetFramesTx_1024B_1518B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTx_1024B_1518B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTx_1024B_1518B"))
	return nil
}

// SetFramesTx_1519B_2047B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTx_1519B_2047B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTx_1519B_2047B"))
	return nil
}

// SetFramesTx_2048B_4095B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTx_2048B_4095B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTx_2048B_4095B"))
	return nil
}

// SetFramesTx_4096B_8191B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTx_4096B_8191B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTx_4096B_8191B"))
	return nil
}

// SetFramesTx_8192B_9215B sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTx_8192B_9215B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTx_8192B_9215B"))
	return nil
}

// SetFramesTxOther sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxOther(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxOther"))
	return nil
}

// SetFramesTxPri_0 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxPri_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxPri_0"))
	return nil
}

// SetFramesTxPri_1 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxPri_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxPri_1"))
	return nil
}

// SetFramesTxPri_2 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxPri_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxPri_2"))
	return nil
}

// SetFramesTxPri_3 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxPri_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxPri_3"))
	return nil
}

// SetFramesTxPri_4 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxPri_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxPri_4"))
	return nil
}

// SetFramesTxPri_5 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxPri_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxPri_5"))
	return nil
}

// SetFramesTxPri_6 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxPri_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxPri_6"))
	return nil
}

// SetFramesTxPri_7 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxPri_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxPri_7"))
	return nil
}

// SetFramesRxPri_0 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxPri_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxPri_0"))
	return nil
}

// SetFramesRxPri_1 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxPri_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxPri_1"))
	return nil
}

// SetFramesRxPri_2 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxPri_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxPri_2"))
	return nil
}

// SetFramesRxPri_3 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxPri_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxPri_3"))
	return nil
}

// SetFramesRxPri_4 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxPri_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxPri_4"))
	return nil
}

// SetFramesRxPri_5 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxPri_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxPri_5"))
	return nil
}

// SetFramesRxPri_6 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxPri_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxPri_6"))
	return nil
}

// SetFramesRxPri_7 sets cunter in shared memory
func (mtr *MacMetrics) SetFramesRxPri_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxPri_7"))
	return nil
}

// SetTxPripause_0_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetTxPripause_0_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPripause_0_1UsCount"))
	return nil
}

// SetTxPripause_1_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetTxPripause_1_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPripause_1_1UsCount"))
	return nil
}

// SetTxPripause_2_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetTxPripause_2_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPripause_2_1UsCount"))
	return nil
}

// SetTxPripause_3_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetTxPripause_3_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPripause_3_1UsCount"))
	return nil
}

// SetTxPripause_4_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetTxPripause_4_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPripause_4_1UsCount"))
	return nil
}

// SetTxPripause_5_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetTxPripause_5_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPripause_5_1UsCount"))
	return nil
}

// SetTxPripause_6_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetTxPripause_6_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPripause_6_1UsCount"))
	return nil
}

// SetTxPripause_7_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetTxPripause_7_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPripause_7_1UsCount"))
	return nil
}

// SetRxPripause_0_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetRxPripause_0_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPripause_0_1UsCount"))
	return nil
}

// SetRxPripause_1_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetRxPripause_1_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPripause_1_1UsCount"))
	return nil
}

// SetRxPripause_2_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetRxPripause_2_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPripause_2_1UsCount"))
	return nil
}

// SetRxPripause_3_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetRxPripause_3_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPripause_3_1UsCount"))
	return nil
}

// SetRxPripause_4_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetRxPripause_4_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPripause_4_1UsCount"))
	return nil
}

// SetRxPripause_5_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetRxPripause_5_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPripause_5_1UsCount"))
	return nil
}

// SetRxPripause_6_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetRxPripause_6_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPripause_6_1UsCount"))
	return nil
}

// SetRxPripause_7_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetRxPripause_7_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPripause_7_1UsCount"))
	return nil
}

// SetRxPause_1UsCount sets cunter in shared memory
func (mtr *MacMetrics) SetRxPause_1UsCount(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPause_1UsCount"))
	return nil
}

// SetFramesTxTruncated sets cunter in shared memory
func (mtr *MacMetrics) SetFramesTxTruncated(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxTruncated"))
	return nil
}

// SetTxPps sets cunter in shared memory
func (mtr *MacMetrics) SetTxPps(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPps"))
	return nil
}

// SetTxBytesps sets cunter in shared memory
func (mtr *MacMetrics) SetTxBytesps(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxBytesps"))
	return nil
}

// SetRxPps sets cunter in shared memory
func (mtr *MacMetrics) SetRxPps(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPps"))
	return nil
}

// SetRxBytesps sets cunter in shared memory
func (mtr *MacMetrics) SetRxBytesps(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxBytesps"))
	return nil
}

// MacMetricsIterator is the iterator object
type MacMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *MacMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *MacMetricsIterator) Next() *MacMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &MacMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *MacMetricsIterator) Find(key uint32) (*MacMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &MacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *MacMetricsIterator) Create(key uint32) (*MacMetrics, error) {
	tmtr := &MacMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &MacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *MacMetricsIterator) Delete(key uint32) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *MacMetricsIterator) Free() {
	it.iter.Free()
}

// NewMacMetricsIterator returns an iterator
func NewMacMetricsIterator() (*MacMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("MacMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &MacMetricsIterator{iter: iter}, nil
}

type MgmtMacMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint32

	FramesRxOk metrics.Counter

	FramesRxAll metrics.Counter

	FramesRxBadFcs metrics.Counter

	FramesRxBadAll metrics.Counter

	OctetsRxOk metrics.Counter

	OctetsRxAll metrics.Counter

	FramesRxUnicast metrics.Counter

	FramesRxMulticast metrics.Counter

	FramesRxBroadcast metrics.Counter

	FramesRxPause metrics.Counter

	FramesRxBadLength metrics.Counter

	FramesRxUndersized metrics.Counter

	FramesRxOversized metrics.Counter

	FramesRxFragments metrics.Counter

	FramesRxJabber metrics.Counter

	FramesRx_64B metrics.Counter

	FramesRx_65B_127B metrics.Counter

	FramesRx_128B_255B metrics.Counter

	FramesRx_256B_511B metrics.Counter

	FramesRx_512B_1023B metrics.Counter

	FramesRx_1024B_1518B metrics.Counter

	FramesRxGt_1518B metrics.Counter

	FramesRxFifoFull metrics.Counter

	FramesTxOk metrics.Counter

	FramesTxAll metrics.Counter

	FramesTxBad metrics.Counter

	OctetsTxOk metrics.Counter

	OctetsTxTotal metrics.Counter

	FramesTxUnicast metrics.Counter

	FramesTxMulticast metrics.Counter

	FramesTxBroadcast metrics.Counter

	FramesTxPause metrics.Counter

	TxPps metrics.Counter

	TxBytesps metrics.Counter

	RxPps metrics.Counter

	RxBytesps metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *MgmtMacMetrics) GetKey() uint32 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *MgmtMacMetrics) Size() int {
	sz := 0

	sz += mtr.FramesRxOk.Size()

	sz += mtr.FramesRxAll.Size()

	sz += mtr.FramesRxBadFcs.Size()

	sz += mtr.FramesRxBadAll.Size()

	sz += mtr.OctetsRxOk.Size()

	sz += mtr.OctetsRxAll.Size()

	sz += mtr.FramesRxUnicast.Size()

	sz += mtr.FramesRxMulticast.Size()

	sz += mtr.FramesRxBroadcast.Size()

	sz += mtr.FramesRxPause.Size()

	sz += mtr.FramesRxBadLength.Size()

	sz += mtr.FramesRxUndersized.Size()

	sz += mtr.FramesRxOversized.Size()

	sz += mtr.FramesRxFragments.Size()

	sz += mtr.FramesRxJabber.Size()

	sz += mtr.FramesRx_64B.Size()

	sz += mtr.FramesRx_65B_127B.Size()

	sz += mtr.FramesRx_128B_255B.Size()

	sz += mtr.FramesRx_256B_511B.Size()

	sz += mtr.FramesRx_512B_1023B.Size()

	sz += mtr.FramesRx_1024B_1518B.Size()

	sz += mtr.FramesRxGt_1518B.Size()

	sz += mtr.FramesRxFifoFull.Size()

	sz += mtr.FramesTxOk.Size()

	sz += mtr.FramesTxAll.Size()

	sz += mtr.FramesTxBad.Size()

	sz += mtr.OctetsTxOk.Size()

	sz += mtr.OctetsTxTotal.Size()

	sz += mtr.FramesTxUnicast.Size()

	sz += mtr.FramesTxMulticast.Size()

	sz += mtr.FramesTxBroadcast.Size()

	sz += mtr.FramesTxPause.Size()

	sz += mtr.TxPps.Size()

	sz += mtr.TxBytesps.Size()

	sz += mtr.RxPps.Size()

	sz += mtr.RxBytesps.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *MgmtMacMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.FramesRxOk = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxOk.Size()

	mtr.FramesRxAll = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxAll.Size()

	mtr.FramesRxBadFcs = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxBadFcs.Size()

	mtr.FramesRxBadAll = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxBadAll.Size()

	mtr.OctetsRxOk = mtr.metrics.GetCounter(offset)
	offset += mtr.OctetsRxOk.Size()

	mtr.OctetsRxAll = mtr.metrics.GetCounter(offset)
	offset += mtr.OctetsRxAll.Size()

	mtr.FramesRxUnicast = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxUnicast.Size()

	mtr.FramesRxMulticast = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxMulticast.Size()

	mtr.FramesRxBroadcast = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxBroadcast.Size()

	mtr.FramesRxPause = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxPause.Size()

	mtr.FramesRxBadLength = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxBadLength.Size()

	mtr.FramesRxUndersized = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxUndersized.Size()

	mtr.FramesRxOversized = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxOversized.Size()

	mtr.FramesRxFragments = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxFragments.Size()

	mtr.FramesRxJabber = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxJabber.Size()

	mtr.FramesRx_64B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_64B.Size()

	mtr.FramesRx_65B_127B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_65B_127B.Size()

	mtr.FramesRx_128B_255B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_128B_255B.Size()

	mtr.FramesRx_256B_511B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_256B_511B.Size()

	mtr.FramesRx_512B_1023B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_512B_1023B.Size()

	mtr.FramesRx_1024B_1518B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRx_1024B_1518B.Size()

	mtr.FramesRxGt_1518B = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxGt_1518B.Size()

	mtr.FramesRxFifoFull = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesRxFifoFull.Size()

	mtr.FramesTxOk = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxOk.Size()

	mtr.FramesTxAll = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxAll.Size()

	mtr.FramesTxBad = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxBad.Size()

	mtr.OctetsTxOk = mtr.metrics.GetCounter(offset)
	offset += mtr.OctetsTxOk.Size()

	mtr.OctetsTxTotal = mtr.metrics.GetCounter(offset)
	offset += mtr.OctetsTxTotal.Size()

	mtr.FramesTxUnicast = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxUnicast.Size()

	mtr.FramesTxMulticast = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxMulticast.Size()

	mtr.FramesTxBroadcast = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxBroadcast.Size()

	mtr.FramesTxPause = mtr.metrics.GetCounter(offset)
	offset += mtr.FramesTxPause.Size()

	mtr.TxPps = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPps.Size()

	mtr.TxBytesps = mtr.metrics.GetCounter(offset)
	offset += mtr.TxBytesps.Size()

	mtr.RxPps = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPps.Size()

	mtr.RxBytesps = mtr.metrics.GetCounter(offset)
	offset += mtr.RxBytesps.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *MgmtMacMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "FramesRxOk" {
		return offset
	}
	offset += mtr.FramesRxOk.Size()

	if fldName == "FramesRxAll" {
		return offset
	}
	offset += mtr.FramesRxAll.Size()

	if fldName == "FramesRxBadFcs" {
		return offset
	}
	offset += mtr.FramesRxBadFcs.Size()

	if fldName == "FramesRxBadAll" {
		return offset
	}
	offset += mtr.FramesRxBadAll.Size()

	if fldName == "OctetsRxOk" {
		return offset
	}
	offset += mtr.OctetsRxOk.Size()

	if fldName == "OctetsRxAll" {
		return offset
	}
	offset += mtr.OctetsRxAll.Size()

	if fldName == "FramesRxUnicast" {
		return offset
	}
	offset += mtr.FramesRxUnicast.Size()

	if fldName == "FramesRxMulticast" {
		return offset
	}
	offset += mtr.FramesRxMulticast.Size()

	if fldName == "FramesRxBroadcast" {
		return offset
	}
	offset += mtr.FramesRxBroadcast.Size()

	if fldName == "FramesRxPause" {
		return offset
	}
	offset += mtr.FramesRxPause.Size()

	if fldName == "FramesRxBadLength" {
		return offset
	}
	offset += mtr.FramesRxBadLength.Size()

	if fldName == "FramesRxUndersized" {
		return offset
	}
	offset += mtr.FramesRxUndersized.Size()

	if fldName == "FramesRxOversized" {
		return offset
	}
	offset += mtr.FramesRxOversized.Size()

	if fldName == "FramesRxFragments" {
		return offset
	}
	offset += mtr.FramesRxFragments.Size()

	if fldName == "FramesRxJabber" {
		return offset
	}
	offset += mtr.FramesRxJabber.Size()

	if fldName == "FramesRx_64B" {
		return offset
	}
	offset += mtr.FramesRx_64B.Size()

	if fldName == "FramesRx_65B_127B" {
		return offset
	}
	offset += mtr.FramesRx_65B_127B.Size()

	if fldName == "FramesRx_128B_255B" {
		return offset
	}
	offset += mtr.FramesRx_128B_255B.Size()

	if fldName == "FramesRx_256B_511B" {
		return offset
	}
	offset += mtr.FramesRx_256B_511B.Size()

	if fldName == "FramesRx_512B_1023B" {
		return offset
	}
	offset += mtr.FramesRx_512B_1023B.Size()

	if fldName == "FramesRx_1024B_1518B" {
		return offset
	}
	offset += mtr.FramesRx_1024B_1518B.Size()

	if fldName == "FramesRxGt_1518B" {
		return offset
	}
	offset += mtr.FramesRxGt_1518B.Size()

	if fldName == "FramesRxFifoFull" {
		return offset
	}
	offset += mtr.FramesRxFifoFull.Size()

	if fldName == "FramesTxOk" {
		return offset
	}
	offset += mtr.FramesTxOk.Size()

	if fldName == "FramesTxAll" {
		return offset
	}
	offset += mtr.FramesTxAll.Size()

	if fldName == "FramesTxBad" {
		return offset
	}
	offset += mtr.FramesTxBad.Size()

	if fldName == "OctetsTxOk" {
		return offset
	}
	offset += mtr.OctetsTxOk.Size()

	if fldName == "OctetsTxTotal" {
		return offset
	}
	offset += mtr.OctetsTxTotal.Size()

	if fldName == "FramesTxUnicast" {
		return offset
	}
	offset += mtr.FramesTxUnicast.Size()

	if fldName == "FramesTxMulticast" {
		return offset
	}
	offset += mtr.FramesTxMulticast.Size()

	if fldName == "FramesTxBroadcast" {
		return offset
	}
	offset += mtr.FramesTxBroadcast.Size()

	if fldName == "FramesTxPause" {
		return offset
	}
	offset += mtr.FramesTxPause.Size()

	if fldName == "TxPps" {
		return offset
	}
	offset += mtr.TxPps.Size()

	if fldName == "TxBytesps" {
		return offset
	}
	offset += mtr.TxBytesps.Size()

	if fldName == "RxPps" {
		return offset
	}
	offset += mtr.RxPps.Size()

	if fldName == "RxBytesps" {
		return offset
	}
	offset += mtr.RxBytesps.Size()

	return offset
}

// SetFramesRxOk sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxOk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxOk"))
	return nil
}

// SetFramesRxAll sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxAll(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxAll"))
	return nil
}

// SetFramesRxBadFcs sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxBadFcs(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxBadFcs"))
	return nil
}

// SetFramesRxBadAll sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxBadAll(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxBadAll"))
	return nil
}

// SetOctetsRxOk sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetOctetsRxOk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OctetsRxOk"))
	return nil
}

// SetOctetsRxAll sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetOctetsRxAll(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OctetsRxAll"))
	return nil
}

// SetFramesRxUnicast sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxUnicast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxUnicast"))
	return nil
}

// SetFramesRxMulticast sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxMulticast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxMulticast"))
	return nil
}

// SetFramesRxBroadcast sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxBroadcast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxBroadcast"))
	return nil
}

// SetFramesRxPause sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxPause(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxPause"))
	return nil
}

// SetFramesRxBadLength sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxBadLength(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxBadLength"))
	return nil
}

// SetFramesRxUndersized sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxUndersized(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxUndersized"))
	return nil
}

// SetFramesRxOversized sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxOversized(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxOversized"))
	return nil
}

// SetFramesRxFragments sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxFragments(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxFragments"))
	return nil
}

// SetFramesRxJabber sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxJabber(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxJabber"))
	return nil
}

// SetFramesRx_64B sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRx_64B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_64B"))
	return nil
}

// SetFramesRx_65B_127B sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRx_65B_127B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_65B_127B"))
	return nil
}

// SetFramesRx_128B_255B sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRx_128B_255B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_128B_255B"))
	return nil
}

// SetFramesRx_256B_511B sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRx_256B_511B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_256B_511B"))
	return nil
}

// SetFramesRx_512B_1023B sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRx_512B_1023B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_512B_1023B"))
	return nil
}

// SetFramesRx_1024B_1518B sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRx_1024B_1518B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRx_1024B_1518B"))
	return nil
}

// SetFramesRxGt_1518B sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxGt_1518B(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxGt_1518B"))
	return nil
}

// SetFramesRxFifoFull sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesRxFifoFull(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesRxFifoFull"))
	return nil
}

// SetFramesTxOk sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesTxOk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxOk"))
	return nil
}

// SetFramesTxAll sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesTxAll(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxAll"))
	return nil
}

// SetFramesTxBad sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesTxBad(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxBad"))
	return nil
}

// SetOctetsTxOk sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetOctetsTxOk(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OctetsTxOk"))
	return nil
}

// SetOctetsTxTotal sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetOctetsTxTotal(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OctetsTxTotal"))
	return nil
}

// SetFramesTxUnicast sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesTxUnicast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxUnicast"))
	return nil
}

// SetFramesTxMulticast sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesTxMulticast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxMulticast"))
	return nil
}

// SetFramesTxBroadcast sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesTxBroadcast(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxBroadcast"))
	return nil
}

// SetFramesTxPause sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetFramesTxPause(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramesTxPause"))
	return nil
}

// SetTxPps sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetTxPps(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPps"))
	return nil
}

// SetTxBytesps sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetTxBytesps(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxBytesps"))
	return nil
}

// SetRxPps sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetRxPps(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPps"))
	return nil
}

// SetRxBytesps sets cunter in shared memory
func (mtr *MgmtMacMetrics) SetRxBytesps(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxBytesps"))
	return nil
}

// MgmtMacMetricsIterator is the iterator object
type MgmtMacMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *MgmtMacMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *MgmtMacMetricsIterator) Next() *MgmtMacMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &MgmtMacMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *MgmtMacMetricsIterator) Find(key uint32) (*MgmtMacMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &MgmtMacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *MgmtMacMetricsIterator) Create(key uint32) (*MgmtMacMetrics, error) {
	tmtr := &MgmtMacMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &MgmtMacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *MgmtMacMetricsIterator) Delete(key uint32) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *MgmtMacMetricsIterator) Free() {
	it.iter.Free()
}

// NewMgmtMacMetricsIterator returns an iterator
func NewMgmtMacMetricsIterator() (*MgmtMacMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("MgmtMacMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &MgmtMacMetricsIterator{iter: iter}, nil
}
