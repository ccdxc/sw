// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/tsdb/metrics"
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

	XRsvd__0 metrics.Counter `structs:"-"`

	XRsvd__1 metrics.Counter `structs:"-"`

	RxDropUnicastBytes metrics.Counter

	RxDropUnicastPackets metrics.Counter

	RxDropMulticastBytes metrics.Counter

	RxDropMulticastPackets metrics.Counter

	RxDropBroadcastBytes metrics.Counter

	RxDropBroadcastPackets metrics.Counter

	RxDmaError metrics.Counter `structs:"-"`

	XRsvd__2 metrics.Counter `structs:"-"`

	TxUnicastBytes metrics.Counter

	TxUnicastPackets metrics.Counter

	TxMulticastBytes metrics.Counter

	TxMulticastPackets metrics.Counter

	TxBroadcastBytes metrics.Counter

	TxBroadcastPackets metrics.Counter

	XRsvd__3 metrics.Counter `structs:"-"`

	XRsvd__4 metrics.Counter `structs:"-"`

	TxDropUnicastBytes metrics.Counter

	TxDropUnicastPackets metrics.Counter

	TxDropMulticastBytes metrics.Counter

	TxDropMulticastPackets metrics.Counter

	TxDropBroadcastBytes metrics.Counter

	TxDropBroadcastPackets metrics.Counter

	TxDmaError metrics.Counter `structs:"-"`

	XRsvd__5 metrics.Counter `structs:"-"`

	RxQueueDisabledDrops metrics.Counter `structs:"-"`

	RxQueueEmptyDrops metrics.Counter `structs:"-"`

	RxQueueSched metrics.Counter `structs:"-"`

	RxDescFetchError metrics.Counter `structs:"-"`

	RxDescDataError metrics.Counter `structs:"-"`

	XRsvd__6 metrics.Counter `structs:"-"`

	XRsvd__7 metrics.Counter `structs:"-"`

	XRsvd__8 metrics.Counter `structs:"-"`

	TxQueueDisabledDrops metrics.Counter `structs:"-"`

	TxQueueSched metrics.Counter `structs:"-"`

	TxDescFetchError metrics.Counter `structs:"-"`

	TxDescDataError metrics.Counter `structs:"-"`

	XRsvd__9 metrics.Counter `structs:"-"`

	XRsvd__10 metrics.Counter `structs:"-"`

	XRsvd__11 metrics.Counter `structs:"-"`

	XRsvd__12 metrics.Counter `structs:"-"`

	TxRdmaUcastBytes metrics.Counter `structs:"-"`

	TxRdmaUcastPackets metrics.Counter `structs:"-"`

	TxRdmaMcastBytes metrics.Counter `structs:"-"`

	TxRdmaMcastPackets metrics.Counter `structs:"-"`

	TxRdmaCnpPackets metrics.Counter `structs:"-"`

	XRsvd__13 metrics.Counter `structs:"-"`

	XRsvd__14 metrics.Counter `structs:"-"`

	XRsvd__15 metrics.Counter `structs:"-"`

	RxRdmaUcastBytes metrics.Counter `structs:"-"`

	RxRdmaUcastPackets metrics.Counter `structs:"-"`

	RxRdmaMcastBytes metrics.Counter `structs:"-"`

	RxRdmaMcastPackets metrics.Counter `structs:"-"`

	RxRdmaCnpPackets metrics.Counter `structs:"-"`

	RxRdmaEcnPackets metrics.Counter `structs:"-"`

	Rsvd16 metrics.Counter `structs:"-"`

	Rsvd17 metrics.Counter `structs:"-"`

	Rsvd18 metrics.Counter `structs:"-"`

	Rsvd19 metrics.Counter `structs:"-"`

	Rsvd20 metrics.Counter `structs:"-"`

	Rsvd21 metrics.Counter `structs:"-"`

	Rsvd22 metrics.Counter `structs:"-"`

	Rsvd23 metrics.Counter `structs:"-"`

	Rsvd24 metrics.Counter `structs:"-"`

	Rsvd25 metrics.Counter `structs:"-"`

	Rsvd26 metrics.Counter `structs:"-"`

	Rsvd27 metrics.Counter `structs:"-"`

	Rsvd28 metrics.Counter `structs:"-"`

	Rsvd29 metrics.Counter `structs:"-"`

	Rsvd30 metrics.Counter `structs:"-"`

	Rsvd31 metrics.Counter `structs:"-"`

	Rsvd32 metrics.Counter `structs:"-"`

	Rsvd33 metrics.Counter `structs:"-"`

	Rsvd34 metrics.Counter `structs:"-"`

	Rsvd35 metrics.Counter `structs:"-"`

	Rsvd36 metrics.Counter `structs:"-"`

	Rsvd37 metrics.Counter `structs:"-"`

	Rsvd38 metrics.Counter `structs:"-"`

	Rsvd39 metrics.Counter `structs:"-"`

	Rsvd40 metrics.Counter `structs:"-"`

	Rsvd41 metrics.Counter `structs:"-"`

	TxPkts metrics.Counter `structs:"-"`

	TxBytes metrics.Counter `structs:"-"`

	RxPkts metrics.Counter `structs:"-"`

	RxBytes metrics.Counter `structs:"-"`

	TxPps metrics.Counter `structs:"-"`

	TxBytesps metrics.Counter `structs:"-"`

	RxPps metrics.Counter `structs:"-"`

	RxBytesps metrics.Counter `structs:"-"`

	RdmaReqRxPktSeqErr metrics.Counter `structs:"-"`

	RdmaReqRxRnrRetryErr metrics.Counter `structs:"-"`

	RdmaReqRxRemoteAccessErr metrics.Counter `structs:"-"`

	RdmaReqRxRemoteInvReqErr metrics.Counter `structs:"-"`

	RdmaReqRxRemoteOperErr metrics.Counter `structs:"-"`

	RdmaReqRxImpliedNakSeqErr metrics.Counter `structs:"-"`

	RdmaReqRxCqeErr metrics.Counter `structs:"-"`

	RdmaReqRxCqeFlushErr metrics.Counter `structs:"-"`

	RdmaReqRxDupResponses metrics.Counter `structs:"-"`

	RdmaReqRxInvalidPackets metrics.Counter `structs:"-"`

	RdmaReqTxLocalAccessErr metrics.Counter `structs:"-"`

	RdmaReqTxLocalOperErr metrics.Counter `structs:"-"`

	RdmaReqTxMemoryMgmtErr metrics.Counter `structs:"-"`

	Rsvd52 metrics.Counter `structs:"-"`

	Rsvd53 metrics.Counter `structs:"-"`

	Rsvd54 metrics.Counter `structs:"-"`

	RdmaRespRxDupRequests metrics.Counter `structs:"-"`

	RdmaRespRxOutOfBuffer metrics.Counter `structs:"-"`

	RdmaRespRxOutOfSeqPkts metrics.Counter `structs:"-"`

	RdmaRespRxCqeErr metrics.Counter `structs:"-"`

	RdmaRespRxCqeFlushErr metrics.Counter `structs:"-"`

	RdmaRespRxLocalLenErr metrics.Counter `structs:"-"`

	RdmaRespRxInvRequestErr metrics.Counter `structs:"-"`

	RdmaRespRxLocalQpOperErr metrics.Counter `structs:"-"`

	RdmaRespRxOutOfAtomicResource metrics.Counter `structs:"-"`

	RdmaRespTxPktSeqErr metrics.Counter `structs:"-"`

	RdmaRespTxRemoteInvReqErr metrics.Counter `structs:"-"`

	RdmaRespTxRemoteAccessErr metrics.Counter `structs:"-"`

	RdmaRespTxRemoteOperErr metrics.Counter `structs:"-"`

	RdmaRespTxRnrRetryErr metrics.Counter `structs:"-"`

	Rsvd57 metrics.Counter `structs:"-"`

	Rsvd58 metrics.Counter `structs:"-"`

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

	sz += mtr.TxRdmaUcastBytes.Size()

	sz += mtr.TxRdmaUcastPackets.Size()

	sz += mtr.TxRdmaMcastBytes.Size()

	sz += mtr.TxRdmaMcastPackets.Size()

	sz += mtr.TxRdmaCnpPackets.Size()

	sz += mtr.XRsvd__13.Size()

	sz += mtr.XRsvd__14.Size()

	sz += mtr.XRsvd__15.Size()

	sz += mtr.RxRdmaUcastBytes.Size()

	sz += mtr.RxRdmaUcastPackets.Size()

	sz += mtr.RxRdmaMcastBytes.Size()

	sz += mtr.RxRdmaMcastPackets.Size()

	sz += mtr.RxRdmaCnpPackets.Size()

	sz += mtr.RxRdmaEcnPackets.Size()

	sz += mtr.Rsvd16.Size()

	sz += mtr.Rsvd17.Size()

	sz += mtr.Rsvd18.Size()

	sz += mtr.Rsvd19.Size()

	sz += mtr.Rsvd20.Size()

	sz += mtr.Rsvd21.Size()

	sz += mtr.Rsvd22.Size()

	sz += mtr.Rsvd23.Size()

	sz += mtr.Rsvd24.Size()

	sz += mtr.Rsvd25.Size()

	sz += mtr.Rsvd26.Size()

	sz += mtr.Rsvd27.Size()

	sz += mtr.Rsvd28.Size()

	sz += mtr.Rsvd29.Size()

	sz += mtr.Rsvd30.Size()

	sz += mtr.Rsvd31.Size()

	sz += mtr.Rsvd32.Size()

	sz += mtr.Rsvd33.Size()

	sz += mtr.Rsvd34.Size()

	sz += mtr.Rsvd35.Size()

	sz += mtr.Rsvd36.Size()

	sz += mtr.Rsvd37.Size()

	sz += mtr.Rsvd38.Size()

	sz += mtr.Rsvd39.Size()

	sz += mtr.Rsvd40.Size()

	sz += mtr.Rsvd41.Size()

	sz += mtr.TxPkts.Size()

	sz += mtr.TxBytes.Size()

	sz += mtr.RxPkts.Size()

	sz += mtr.RxBytes.Size()

	sz += mtr.TxPps.Size()

	sz += mtr.TxBytesps.Size()

	sz += mtr.RxPps.Size()

	sz += mtr.RxBytesps.Size()

	sz += mtr.RdmaReqRxPktSeqErr.Size()

	sz += mtr.RdmaReqRxRnrRetryErr.Size()

	sz += mtr.RdmaReqRxRemoteAccessErr.Size()

	sz += mtr.RdmaReqRxRemoteInvReqErr.Size()

	sz += mtr.RdmaReqRxRemoteOperErr.Size()

	sz += mtr.RdmaReqRxImpliedNakSeqErr.Size()

	sz += mtr.RdmaReqRxCqeErr.Size()

	sz += mtr.RdmaReqRxCqeFlushErr.Size()

	sz += mtr.RdmaReqRxDupResponses.Size()

	sz += mtr.RdmaReqRxInvalidPackets.Size()

	sz += mtr.RdmaReqTxLocalAccessErr.Size()

	sz += mtr.RdmaReqTxLocalOperErr.Size()

	sz += mtr.RdmaReqTxMemoryMgmtErr.Size()

	sz += mtr.Rsvd52.Size()

	sz += mtr.Rsvd53.Size()

	sz += mtr.Rsvd54.Size()

	sz += mtr.RdmaRespRxDupRequests.Size()

	sz += mtr.RdmaRespRxOutOfBuffer.Size()

	sz += mtr.RdmaRespRxOutOfSeqPkts.Size()

	sz += mtr.RdmaRespRxCqeErr.Size()

	sz += mtr.RdmaRespRxCqeFlushErr.Size()

	sz += mtr.RdmaRespRxLocalLenErr.Size()

	sz += mtr.RdmaRespRxInvRequestErr.Size()

	sz += mtr.RdmaRespRxLocalQpOperErr.Size()

	sz += mtr.RdmaRespRxOutOfAtomicResource.Size()

	sz += mtr.RdmaRespTxPktSeqErr.Size()

	sz += mtr.RdmaRespTxRemoteInvReqErr.Size()

	sz += mtr.RdmaRespTxRemoteAccessErr.Size()

	sz += mtr.RdmaRespTxRemoteOperErr.Size()

	sz += mtr.RdmaRespTxRnrRetryErr.Size()

	sz += mtr.Rsvd57.Size()

	sz += mtr.Rsvd58.Size()

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

	mtr.TxRdmaUcastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxRdmaUcastBytes.Size()

	mtr.TxRdmaUcastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxRdmaUcastPackets.Size()

	mtr.TxRdmaMcastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxRdmaMcastBytes.Size()

	mtr.TxRdmaMcastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxRdmaMcastPackets.Size()

	mtr.TxRdmaCnpPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.TxRdmaCnpPackets.Size()

	mtr.XRsvd__13 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__13.Size()

	mtr.XRsvd__14 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__14.Size()

	mtr.XRsvd__15 = mtr.metrics.GetCounter(offset)
	offset += mtr.XRsvd__15.Size()

	mtr.RxRdmaUcastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.RxRdmaUcastBytes.Size()

	mtr.RxRdmaUcastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RxRdmaUcastPackets.Size()

	mtr.RxRdmaMcastBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.RxRdmaMcastBytes.Size()

	mtr.RxRdmaMcastPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RxRdmaMcastPackets.Size()

	mtr.RxRdmaCnpPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RxRdmaCnpPackets.Size()

	mtr.RxRdmaEcnPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RxRdmaEcnPackets.Size()

	mtr.Rsvd16 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd16.Size()

	mtr.Rsvd17 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd17.Size()

	mtr.Rsvd18 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd18.Size()

	mtr.Rsvd19 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd19.Size()

	mtr.Rsvd20 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd20.Size()

	mtr.Rsvd21 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd21.Size()

	mtr.Rsvd22 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd22.Size()

	mtr.Rsvd23 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd23.Size()

	mtr.Rsvd24 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd24.Size()

	mtr.Rsvd25 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd25.Size()

	mtr.Rsvd26 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd26.Size()

	mtr.Rsvd27 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd27.Size()

	mtr.Rsvd28 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd28.Size()

	mtr.Rsvd29 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd29.Size()

	mtr.Rsvd30 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd30.Size()

	mtr.Rsvd31 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd31.Size()

	mtr.Rsvd32 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd32.Size()

	mtr.Rsvd33 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd33.Size()

	mtr.Rsvd34 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd34.Size()

	mtr.Rsvd35 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd35.Size()

	mtr.Rsvd36 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd36.Size()

	mtr.Rsvd37 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd37.Size()

	mtr.Rsvd38 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd38.Size()

	mtr.Rsvd39 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd39.Size()

	mtr.Rsvd40 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd40.Size()

	mtr.Rsvd41 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd41.Size()

	mtr.TxPkts = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPkts.Size()

	mtr.TxBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.TxBytes.Size()

	mtr.RxPkts = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPkts.Size()

	mtr.RxBytes = mtr.metrics.GetCounter(offset)
	offset += mtr.RxBytes.Size()

	mtr.TxPps = mtr.metrics.GetCounter(offset)
	offset += mtr.TxPps.Size()

	mtr.TxBytesps = mtr.metrics.GetCounter(offset)
	offset += mtr.TxBytesps.Size()

	mtr.RxPps = mtr.metrics.GetCounter(offset)
	offset += mtr.RxPps.Size()

	mtr.RxBytesps = mtr.metrics.GetCounter(offset)
	offset += mtr.RxBytesps.Size()

	mtr.RdmaReqRxPktSeqErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaReqRxPktSeqErr.Size()

	mtr.RdmaReqRxRnrRetryErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaReqRxRnrRetryErr.Size()

	mtr.RdmaReqRxRemoteAccessErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaReqRxRemoteAccessErr.Size()

	mtr.RdmaReqRxRemoteInvReqErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaReqRxRemoteInvReqErr.Size()

	mtr.RdmaReqRxRemoteOperErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaReqRxRemoteOperErr.Size()

	mtr.RdmaReqRxImpliedNakSeqErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaReqRxImpliedNakSeqErr.Size()

	mtr.RdmaReqRxCqeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaReqRxCqeErr.Size()

	mtr.RdmaReqRxCqeFlushErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaReqRxCqeFlushErr.Size()

	mtr.RdmaReqRxDupResponses = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaReqRxDupResponses.Size()

	mtr.RdmaReqRxInvalidPackets = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaReqRxInvalidPackets.Size()

	mtr.RdmaReqTxLocalAccessErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaReqTxLocalAccessErr.Size()

	mtr.RdmaReqTxLocalOperErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaReqTxLocalOperErr.Size()

	mtr.RdmaReqTxMemoryMgmtErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaReqTxMemoryMgmtErr.Size()

	mtr.Rsvd52 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd52.Size()

	mtr.Rsvd53 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd53.Size()

	mtr.Rsvd54 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd54.Size()

	mtr.RdmaRespRxDupRequests = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespRxDupRequests.Size()

	mtr.RdmaRespRxOutOfBuffer = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespRxOutOfBuffer.Size()

	mtr.RdmaRespRxOutOfSeqPkts = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespRxOutOfSeqPkts.Size()

	mtr.RdmaRespRxCqeErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespRxCqeErr.Size()

	mtr.RdmaRespRxCqeFlushErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespRxCqeFlushErr.Size()

	mtr.RdmaRespRxLocalLenErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespRxLocalLenErr.Size()

	mtr.RdmaRespRxInvRequestErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespRxInvRequestErr.Size()

	mtr.RdmaRespRxLocalQpOperErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespRxLocalQpOperErr.Size()

	mtr.RdmaRespRxOutOfAtomicResource = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespRxOutOfAtomicResource.Size()

	mtr.RdmaRespTxPktSeqErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespTxPktSeqErr.Size()

	mtr.RdmaRespTxRemoteInvReqErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespTxRemoteInvReqErr.Size()

	mtr.RdmaRespTxRemoteAccessErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespTxRemoteAccessErr.Size()

	mtr.RdmaRespTxRemoteOperErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespTxRemoteOperErr.Size()

	mtr.RdmaRespTxRnrRetryErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RdmaRespTxRnrRetryErr.Size()

	mtr.Rsvd57 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd57.Size()

	mtr.Rsvd58 = mtr.metrics.GetCounter(offset)
	offset += mtr.Rsvd58.Size()

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

	if fldName == "TxRdmaUcastBytes" {
		return offset
	}
	offset += mtr.TxRdmaUcastBytes.Size()

	if fldName == "TxRdmaUcastPackets" {
		return offset
	}
	offset += mtr.TxRdmaUcastPackets.Size()

	if fldName == "TxRdmaMcastBytes" {
		return offset
	}
	offset += mtr.TxRdmaMcastBytes.Size()

	if fldName == "TxRdmaMcastPackets" {
		return offset
	}
	offset += mtr.TxRdmaMcastPackets.Size()

	if fldName == "TxRdmaCnpPackets" {
		return offset
	}
	offset += mtr.TxRdmaCnpPackets.Size()

	if fldName == "XRsvd__13" {
		return offset
	}
	offset += mtr.XRsvd__13.Size()

	if fldName == "XRsvd__14" {
		return offset
	}
	offset += mtr.XRsvd__14.Size()

	if fldName == "XRsvd__15" {
		return offset
	}
	offset += mtr.XRsvd__15.Size()

	if fldName == "RxRdmaUcastBytes" {
		return offset
	}
	offset += mtr.RxRdmaUcastBytes.Size()

	if fldName == "RxRdmaUcastPackets" {
		return offset
	}
	offset += mtr.RxRdmaUcastPackets.Size()

	if fldName == "RxRdmaMcastBytes" {
		return offset
	}
	offset += mtr.RxRdmaMcastBytes.Size()

	if fldName == "RxRdmaMcastPackets" {
		return offset
	}
	offset += mtr.RxRdmaMcastPackets.Size()

	if fldName == "RxRdmaCnpPackets" {
		return offset
	}
	offset += mtr.RxRdmaCnpPackets.Size()

	if fldName == "RxRdmaEcnPackets" {
		return offset
	}
	offset += mtr.RxRdmaEcnPackets.Size()

	if fldName == "Rsvd16" {
		return offset
	}
	offset += mtr.Rsvd16.Size()

	if fldName == "Rsvd17" {
		return offset
	}
	offset += mtr.Rsvd17.Size()

	if fldName == "Rsvd18" {
		return offset
	}
	offset += mtr.Rsvd18.Size()

	if fldName == "Rsvd19" {
		return offset
	}
	offset += mtr.Rsvd19.Size()

	if fldName == "Rsvd20" {
		return offset
	}
	offset += mtr.Rsvd20.Size()

	if fldName == "Rsvd21" {
		return offset
	}
	offset += mtr.Rsvd21.Size()

	if fldName == "Rsvd22" {
		return offset
	}
	offset += mtr.Rsvd22.Size()

	if fldName == "Rsvd23" {
		return offset
	}
	offset += mtr.Rsvd23.Size()

	if fldName == "Rsvd24" {
		return offset
	}
	offset += mtr.Rsvd24.Size()

	if fldName == "Rsvd25" {
		return offset
	}
	offset += mtr.Rsvd25.Size()

	if fldName == "Rsvd26" {
		return offset
	}
	offset += mtr.Rsvd26.Size()

	if fldName == "Rsvd27" {
		return offset
	}
	offset += mtr.Rsvd27.Size()

	if fldName == "Rsvd28" {
		return offset
	}
	offset += mtr.Rsvd28.Size()

	if fldName == "Rsvd29" {
		return offset
	}
	offset += mtr.Rsvd29.Size()

	if fldName == "Rsvd30" {
		return offset
	}
	offset += mtr.Rsvd30.Size()

	if fldName == "Rsvd31" {
		return offset
	}
	offset += mtr.Rsvd31.Size()

	if fldName == "Rsvd32" {
		return offset
	}
	offset += mtr.Rsvd32.Size()

	if fldName == "Rsvd33" {
		return offset
	}
	offset += mtr.Rsvd33.Size()

	if fldName == "Rsvd34" {
		return offset
	}
	offset += mtr.Rsvd34.Size()

	if fldName == "Rsvd35" {
		return offset
	}
	offset += mtr.Rsvd35.Size()

	if fldName == "Rsvd36" {
		return offset
	}
	offset += mtr.Rsvd36.Size()

	if fldName == "Rsvd37" {
		return offset
	}
	offset += mtr.Rsvd37.Size()

	if fldName == "Rsvd38" {
		return offset
	}
	offset += mtr.Rsvd38.Size()

	if fldName == "Rsvd39" {
		return offset
	}
	offset += mtr.Rsvd39.Size()

	if fldName == "Rsvd40" {
		return offset
	}
	offset += mtr.Rsvd40.Size()

	if fldName == "Rsvd41" {
		return offset
	}
	offset += mtr.Rsvd41.Size()

	if fldName == "TxPkts" {
		return offset
	}
	offset += mtr.TxPkts.Size()

	if fldName == "TxBytes" {
		return offset
	}
	offset += mtr.TxBytes.Size()

	if fldName == "RxPkts" {
		return offset
	}
	offset += mtr.RxPkts.Size()

	if fldName == "RxBytes" {
		return offset
	}
	offset += mtr.RxBytes.Size()

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

	if fldName == "RdmaReqRxPktSeqErr" {
		return offset
	}
	offset += mtr.RdmaReqRxPktSeqErr.Size()

	if fldName == "RdmaReqRxRnrRetryErr" {
		return offset
	}
	offset += mtr.RdmaReqRxRnrRetryErr.Size()

	if fldName == "RdmaReqRxRemoteAccessErr" {
		return offset
	}
	offset += mtr.RdmaReqRxRemoteAccessErr.Size()

	if fldName == "RdmaReqRxRemoteInvReqErr" {
		return offset
	}
	offset += mtr.RdmaReqRxRemoteInvReqErr.Size()

	if fldName == "RdmaReqRxRemoteOperErr" {
		return offset
	}
	offset += mtr.RdmaReqRxRemoteOperErr.Size()

	if fldName == "RdmaReqRxImpliedNakSeqErr" {
		return offset
	}
	offset += mtr.RdmaReqRxImpliedNakSeqErr.Size()

	if fldName == "RdmaReqRxCqeErr" {
		return offset
	}
	offset += mtr.RdmaReqRxCqeErr.Size()

	if fldName == "RdmaReqRxCqeFlushErr" {
		return offset
	}
	offset += mtr.RdmaReqRxCqeFlushErr.Size()

	if fldName == "RdmaReqRxDupResponses" {
		return offset
	}
	offset += mtr.RdmaReqRxDupResponses.Size()

	if fldName == "RdmaReqRxInvalidPackets" {
		return offset
	}
	offset += mtr.RdmaReqRxInvalidPackets.Size()

	if fldName == "RdmaReqTxLocalAccessErr" {
		return offset
	}
	offset += mtr.RdmaReqTxLocalAccessErr.Size()

	if fldName == "RdmaReqTxLocalOperErr" {
		return offset
	}
	offset += mtr.RdmaReqTxLocalOperErr.Size()

	if fldName == "RdmaReqTxMemoryMgmtErr" {
		return offset
	}
	offset += mtr.RdmaReqTxMemoryMgmtErr.Size()

	if fldName == "Rsvd52" {
		return offset
	}
	offset += mtr.Rsvd52.Size()

	if fldName == "Rsvd53" {
		return offset
	}
	offset += mtr.Rsvd53.Size()

	if fldName == "Rsvd54" {
		return offset
	}
	offset += mtr.Rsvd54.Size()

	if fldName == "RdmaRespRxDupRequests" {
		return offset
	}
	offset += mtr.RdmaRespRxDupRequests.Size()

	if fldName == "RdmaRespRxOutOfBuffer" {
		return offset
	}
	offset += mtr.RdmaRespRxOutOfBuffer.Size()

	if fldName == "RdmaRespRxOutOfSeqPkts" {
		return offset
	}
	offset += mtr.RdmaRespRxOutOfSeqPkts.Size()

	if fldName == "RdmaRespRxCqeErr" {
		return offset
	}
	offset += mtr.RdmaRespRxCqeErr.Size()

	if fldName == "RdmaRespRxCqeFlushErr" {
		return offset
	}
	offset += mtr.RdmaRespRxCqeFlushErr.Size()

	if fldName == "RdmaRespRxLocalLenErr" {
		return offset
	}
	offset += mtr.RdmaRespRxLocalLenErr.Size()

	if fldName == "RdmaRespRxInvRequestErr" {
		return offset
	}
	offset += mtr.RdmaRespRxInvRequestErr.Size()

	if fldName == "RdmaRespRxLocalQpOperErr" {
		return offset
	}
	offset += mtr.RdmaRespRxLocalQpOperErr.Size()

	if fldName == "RdmaRespRxOutOfAtomicResource" {
		return offset
	}
	offset += mtr.RdmaRespRxOutOfAtomicResource.Size()

	if fldName == "RdmaRespTxPktSeqErr" {
		return offset
	}
	offset += mtr.RdmaRespTxPktSeqErr.Size()

	if fldName == "RdmaRespTxRemoteInvReqErr" {
		return offset
	}
	offset += mtr.RdmaRespTxRemoteInvReqErr.Size()

	if fldName == "RdmaRespTxRemoteAccessErr" {
		return offset
	}
	offset += mtr.RdmaRespTxRemoteAccessErr.Size()

	if fldName == "RdmaRespTxRemoteOperErr" {
		return offset
	}
	offset += mtr.RdmaRespTxRemoteOperErr.Size()

	if fldName == "RdmaRespTxRnrRetryErr" {
		return offset
	}
	offset += mtr.RdmaRespTxRnrRetryErr.Size()

	if fldName == "Rsvd57" {
		return offset
	}
	offset += mtr.Rsvd57.Size()

	if fldName == "Rsvd58" {
		return offset
	}
	offset += mtr.Rsvd58.Size()

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

// SetTxRdmaUcastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxRdmaUcastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxRdmaUcastBytes"))
	return nil
}

// SetTxRdmaUcastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxRdmaUcastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxRdmaUcastPackets"))
	return nil
}

// SetTxRdmaMcastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxRdmaMcastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxRdmaMcastBytes"))
	return nil
}

// SetTxRdmaMcastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxRdmaMcastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxRdmaMcastPackets"))
	return nil
}

// SetTxRdmaCnpPackets sets cunter in shared memory
func (mtr *LifMetrics) SetTxRdmaCnpPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxRdmaCnpPackets"))
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

// SetXRsvd__15 sets cunter in shared memory
func (mtr *LifMetrics) SetXRsvd__15(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XRsvd__15"))
	return nil
}

// SetRxRdmaUcastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetRxRdmaUcastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxRdmaUcastBytes"))
	return nil
}

// SetRxRdmaUcastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRxRdmaUcastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxRdmaUcastPackets"))
	return nil
}

// SetRxRdmaMcastBytes sets cunter in shared memory
func (mtr *LifMetrics) SetRxRdmaMcastBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxRdmaMcastBytes"))
	return nil
}

// SetRxRdmaMcastPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRxRdmaMcastPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxRdmaMcastPackets"))
	return nil
}

// SetRxRdmaCnpPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRxRdmaCnpPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxRdmaCnpPackets"))
	return nil
}

// SetRxRdmaEcnPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRxRdmaEcnPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxRdmaEcnPackets"))
	return nil
}

// SetRsvd16 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd16(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd16"))
	return nil
}

// SetRsvd17 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd17(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd17"))
	return nil
}

// SetRsvd18 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd18(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd18"))
	return nil
}

// SetRsvd19 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd19(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd19"))
	return nil
}

// SetRsvd20 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd20(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd20"))
	return nil
}

// SetRsvd21 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd21(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd21"))
	return nil
}

// SetRsvd22 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd22(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd22"))
	return nil
}

// SetRsvd23 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd23(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd23"))
	return nil
}

// SetRsvd24 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd24(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd24"))
	return nil
}

// SetRsvd25 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd25(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd25"))
	return nil
}

// SetRsvd26 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd26(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd26"))
	return nil
}

// SetRsvd27 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd27(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd27"))
	return nil
}

// SetRsvd28 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd28(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd28"))
	return nil
}

// SetRsvd29 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd29(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd29"))
	return nil
}

// SetRsvd30 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd30(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd30"))
	return nil
}

// SetRsvd31 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd31(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd31"))
	return nil
}

// SetRsvd32 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd32(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd32"))
	return nil
}

// SetRsvd33 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd33(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd33"))
	return nil
}

// SetRsvd34 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd34(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd34"))
	return nil
}

// SetRsvd35 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd35(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd35"))
	return nil
}

// SetRsvd36 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd36(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd36"))
	return nil
}

// SetRsvd37 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd37(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd37"))
	return nil
}

// SetRsvd38 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd38(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd38"))
	return nil
}

// SetRsvd39 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd39(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd39"))
	return nil
}

// SetRsvd40 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd40(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd40"))
	return nil
}

// SetRsvd41 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd41(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd41"))
	return nil
}

// SetTxPkts sets cunter in shared memory
func (mtr *LifMetrics) SetTxPkts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPkts"))
	return nil
}

// SetTxBytes sets cunter in shared memory
func (mtr *LifMetrics) SetTxBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxBytes"))
	return nil
}

// SetRxPkts sets cunter in shared memory
func (mtr *LifMetrics) SetRxPkts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPkts"))
	return nil
}

// SetRxBytes sets cunter in shared memory
func (mtr *LifMetrics) SetRxBytes(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxBytes"))
	return nil
}

// SetTxPps sets cunter in shared memory
func (mtr *LifMetrics) SetTxPps(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxPps"))
	return nil
}

// SetTxBytesps sets cunter in shared memory
func (mtr *LifMetrics) SetTxBytesps(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TxBytesps"))
	return nil
}

// SetRxPps sets cunter in shared memory
func (mtr *LifMetrics) SetRxPps(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxPps"))
	return nil
}

// SetRxBytesps sets cunter in shared memory
func (mtr *LifMetrics) SetRxBytesps(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RxBytesps"))
	return nil
}

// SetRdmaReqRxPktSeqErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaReqRxPktSeqErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaReqRxPktSeqErr"))
	return nil
}

// SetRdmaReqRxRnrRetryErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaReqRxRnrRetryErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaReqRxRnrRetryErr"))
	return nil
}

// SetRdmaReqRxRemoteAccessErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaReqRxRemoteAccessErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaReqRxRemoteAccessErr"))
	return nil
}

// SetRdmaReqRxRemoteInvReqErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaReqRxRemoteInvReqErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaReqRxRemoteInvReqErr"))
	return nil
}

// SetRdmaReqRxRemoteOperErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaReqRxRemoteOperErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaReqRxRemoteOperErr"))
	return nil
}

// SetRdmaReqRxImpliedNakSeqErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaReqRxImpliedNakSeqErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaReqRxImpliedNakSeqErr"))
	return nil
}

// SetRdmaReqRxCqeErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaReqRxCqeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaReqRxCqeErr"))
	return nil
}

// SetRdmaReqRxCqeFlushErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaReqRxCqeFlushErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaReqRxCqeFlushErr"))
	return nil
}

// SetRdmaReqRxDupResponses sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaReqRxDupResponses(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaReqRxDupResponses"))
	return nil
}

// SetRdmaReqRxInvalidPackets sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaReqRxInvalidPackets(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaReqRxInvalidPackets"))
	return nil
}

// SetRdmaReqTxLocalAccessErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaReqTxLocalAccessErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaReqTxLocalAccessErr"))
	return nil
}

// SetRdmaReqTxLocalOperErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaReqTxLocalOperErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaReqTxLocalOperErr"))
	return nil
}

// SetRdmaReqTxMemoryMgmtErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaReqTxMemoryMgmtErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaReqTxMemoryMgmtErr"))
	return nil
}

// SetRsvd52 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd52(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd52"))
	return nil
}

// SetRsvd53 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd53(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd53"))
	return nil
}

// SetRsvd54 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd54(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd54"))
	return nil
}

// SetRdmaRespRxDupRequests sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespRxDupRequests(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespRxDupRequests"))
	return nil
}

// SetRdmaRespRxOutOfBuffer sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespRxOutOfBuffer(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespRxOutOfBuffer"))
	return nil
}

// SetRdmaRespRxOutOfSeqPkts sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespRxOutOfSeqPkts(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespRxOutOfSeqPkts"))
	return nil
}

// SetRdmaRespRxCqeErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespRxCqeErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespRxCqeErr"))
	return nil
}

// SetRdmaRespRxCqeFlushErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespRxCqeFlushErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespRxCqeFlushErr"))
	return nil
}

// SetRdmaRespRxLocalLenErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespRxLocalLenErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespRxLocalLenErr"))
	return nil
}

// SetRdmaRespRxInvRequestErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespRxInvRequestErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespRxInvRequestErr"))
	return nil
}

// SetRdmaRespRxLocalQpOperErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespRxLocalQpOperErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespRxLocalQpOperErr"))
	return nil
}

// SetRdmaRespRxOutOfAtomicResource sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespRxOutOfAtomicResource(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespRxOutOfAtomicResource"))
	return nil
}

// SetRdmaRespTxPktSeqErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespTxPktSeqErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespTxPktSeqErr"))
	return nil
}

// SetRdmaRespTxRemoteInvReqErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespTxRemoteInvReqErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespTxRemoteInvReqErr"))
	return nil
}

// SetRdmaRespTxRemoteAccessErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespTxRemoteAccessErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespTxRemoteAccessErr"))
	return nil
}

// SetRdmaRespTxRemoteOperErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespTxRemoteOperErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespTxRemoteOperErr"))
	return nil
}

// SetRdmaRespTxRnrRetryErr sets cunter in shared memory
func (mtr *LifMetrics) SetRdmaRespTxRnrRetryErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RdmaRespTxRnrRetryErr"))
	return nil
}

// SetRsvd57 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd57(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd57"))
	return nil
}

// SetRsvd58 sets cunter in shared memory
func (mtr *LifMetrics) SetRsvd58(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rsvd58"))
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
	if mtr == nil {
		return nil
	}

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
