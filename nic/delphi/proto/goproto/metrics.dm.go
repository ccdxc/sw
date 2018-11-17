// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/golang/protobuf/proto"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
)

type LifMetrics struct {
	key uint64

	rx_unicast_packets gometrics.Counter

	rx_multicast_packets gometrics.Counter

	rx_broadcast_packets gometrics.Counter

	__rsvd__0 gometrics.Counter

	rx_unicast_bytes gometrics.Counter

	rx_broadcast_bytes gometrics.Counter

	rx_multicast_bytes gometrics.Counter

	__rsvd__1 gometrics.Counter

	rx_queue_empty_drops gometrics.Counter

	rx_queue_disabled_drops gometrics.Counter

	rx_desc_error gometrics.Counter

	__rsvd__2 gometrics.Counter

	__rsvd__3 gometrics.Counter

	__rsvd__4 gometrics.Counter

	__rsvd__5 gometrics.Counter

	__rsvd__6 gometrics.Counter

	tx_csum_none gometrics.Counter

	tx_csum_hw gometrics.Counter

	tx_csum_hw_inner gometrics.Counter

	tx_csum_partial gometrics.Counter

	tx_sg gometrics.Counter

	tx_tso gometrics.Counter

	tx_tso_sop gometrics.Counter

	tx_tso_eop gometrics.Counter

	tx_op_error gometrics.Counter

	__rsvd__7 gometrics.Counter

	__rsvd__8 gometrics.Counter

	__rsvd__9 gometrics.Counter

	__rsvd__10 gometrics.Counter

	__rsvd__11 gometrics.Counter

	__rsvd__12 gometrics.Counter

	__rsvd__13 gometrics.Counter

	tx_unicast_packets gometrics.Counter

	tx_unicast_bytes gometrics.Counter

	tx_multicast_packets gometrics.Counter

	tx_multicast_bytes gometrics.Counter

	tx_broadcast_packets gometrics.Counter

	tx_broadcast_bytes gometrics.Counter

	__rsvd__14 gometrics.Counter

	__rsvd__15 gometrics.Counter

	tx_drop_unicast_packets gometrics.Counter

	tx_drop_unicast_bytes gometrics.Counter

	tx_drop_multicast_packets gometrics.Counter

	tx_drop_multicast_bytes gometrics.Counter

	tx_drop_broadcast_packets gometrics.Counter

	tx_drop_broadcast_bytes gometrics.Counter

	tx_dma_error gometrics.Counter

	__rsvd__16 gometrics.Counter

	tx_queue_disabled gometrics.Counter

	tx_queue_sched gometrics.Counter

	tx_desc_error gometrics.Counter

	__rsvd__17 gometrics.Counter

	__rsvd__18 gometrics.Counter

	__rsvd__19 gometrics.Counter

	__rsvd__20 gometrics.Counter

	__rsvd__21 gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *LifMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *LifMetrics) Size() int {
	sz := 0

	sz += mtr.rx_unicast_packets.Size()

	sz += mtr.rx_multicast_packets.Size()

	sz += mtr.rx_broadcast_packets.Size()

	sz += mtr.__rsvd__0.Size()

	sz += mtr.rx_unicast_bytes.Size()

	sz += mtr.rx_broadcast_bytes.Size()

	sz += mtr.rx_multicast_bytes.Size()

	sz += mtr.__rsvd__1.Size()

	sz += mtr.rx_queue_empty_drops.Size()

	sz += mtr.rx_queue_disabled_drops.Size()

	sz += mtr.rx_desc_error.Size()

	sz += mtr.__rsvd__2.Size()

	sz += mtr.__rsvd__3.Size()

	sz += mtr.__rsvd__4.Size()

	sz += mtr.__rsvd__5.Size()

	sz += mtr.__rsvd__6.Size()

	sz += mtr.tx_csum_none.Size()

	sz += mtr.tx_csum_hw.Size()

	sz += mtr.tx_csum_hw_inner.Size()

	sz += mtr.tx_csum_partial.Size()

	sz += mtr.tx_sg.Size()

	sz += mtr.tx_tso.Size()

	sz += mtr.tx_tso_sop.Size()

	sz += mtr.tx_tso_eop.Size()

	sz += mtr.tx_op_error.Size()

	sz += mtr.__rsvd__7.Size()

	sz += mtr.__rsvd__8.Size()

	sz += mtr.__rsvd__9.Size()

	sz += mtr.__rsvd__10.Size()

	sz += mtr.__rsvd__11.Size()

	sz += mtr.__rsvd__12.Size()

	sz += mtr.__rsvd__13.Size()

	sz += mtr.tx_unicast_packets.Size()

	sz += mtr.tx_unicast_bytes.Size()

	sz += mtr.tx_multicast_packets.Size()

	sz += mtr.tx_multicast_bytes.Size()

	sz += mtr.tx_broadcast_packets.Size()

	sz += mtr.tx_broadcast_bytes.Size()

	sz += mtr.__rsvd__14.Size()

	sz += mtr.__rsvd__15.Size()

	sz += mtr.tx_drop_unicast_packets.Size()

	sz += mtr.tx_drop_unicast_bytes.Size()

	sz += mtr.tx_drop_multicast_packets.Size()

	sz += mtr.tx_drop_multicast_bytes.Size()

	sz += mtr.tx_drop_broadcast_packets.Size()

	sz += mtr.tx_drop_broadcast_bytes.Size()

	sz += mtr.tx_dma_error.Size()

	sz += mtr.__rsvd__16.Size()

	sz += mtr.tx_queue_disabled.Size()

	sz += mtr.tx_queue_sched.Size()

	sz += mtr.tx_desc_error.Size()

	sz += mtr.__rsvd__17.Size()

	sz += mtr.__rsvd__18.Size()

	sz += mtr.__rsvd__19.Size()

	sz += mtr.__rsvd__20.Size()

	sz += mtr.__rsvd__21.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *LifMetrics) Unmarshal() error {
	var offset int

	val, _ := proto.DecodeVarint([]byte(mtr.metrics.GetKey()))
	mtr.key = uint64(val)

	mtr.rx_unicast_packets = mtr.metrics.GetCounter(offset)
	offset += mtr.rx_unicast_packets.Size()

	mtr.rx_multicast_packets = mtr.metrics.GetCounter(offset)
	offset += mtr.rx_multicast_packets.Size()

	mtr.rx_broadcast_packets = mtr.metrics.GetCounter(offset)
	offset += mtr.rx_broadcast_packets.Size()

	mtr.__rsvd__0 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__0.Size()

	mtr.rx_unicast_bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.rx_unicast_bytes.Size()

	mtr.rx_broadcast_bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.rx_broadcast_bytes.Size()

	mtr.rx_multicast_bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.rx_multicast_bytes.Size()

	mtr.__rsvd__1 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__1.Size()

	mtr.rx_queue_empty_drops = mtr.metrics.GetCounter(offset)
	offset += mtr.rx_queue_empty_drops.Size()

	mtr.rx_queue_disabled_drops = mtr.metrics.GetCounter(offset)
	offset += mtr.rx_queue_disabled_drops.Size()

	mtr.rx_desc_error = mtr.metrics.GetCounter(offset)
	offset += mtr.rx_desc_error.Size()

	mtr.__rsvd__2 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__2.Size()

	mtr.__rsvd__3 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__3.Size()

	mtr.__rsvd__4 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__4.Size()

	mtr.__rsvd__5 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__5.Size()

	mtr.__rsvd__6 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__6.Size()

	mtr.tx_csum_none = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_csum_none.Size()

	mtr.tx_csum_hw = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_csum_hw.Size()

	mtr.tx_csum_hw_inner = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_csum_hw_inner.Size()

	mtr.tx_csum_partial = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_csum_partial.Size()

	mtr.tx_sg = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_sg.Size()

	mtr.tx_tso = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_tso.Size()

	mtr.tx_tso_sop = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_tso_sop.Size()

	mtr.tx_tso_eop = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_tso_eop.Size()

	mtr.tx_op_error = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_op_error.Size()

	mtr.__rsvd__7 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__7.Size()

	mtr.__rsvd__8 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__8.Size()

	mtr.__rsvd__9 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__9.Size()

	mtr.__rsvd__10 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__10.Size()

	mtr.__rsvd__11 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__11.Size()

	mtr.__rsvd__12 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__12.Size()

	mtr.__rsvd__13 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__13.Size()

	mtr.tx_unicast_packets = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_unicast_packets.Size()

	mtr.tx_unicast_bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_unicast_bytes.Size()

	mtr.tx_multicast_packets = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_multicast_packets.Size()

	mtr.tx_multicast_bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_multicast_bytes.Size()

	mtr.tx_broadcast_packets = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_broadcast_packets.Size()

	mtr.tx_broadcast_bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_broadcast_bytes.Size()

	mtr.__rsvd__14 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__14.Size()

	mtr.__rsvd__15 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__15.Size()

	mtr.tx_drop_unicast_packets = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_drop_unicast_packets.Size()

	mtr.tx_drop_unicast_bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_drop_unicast_bytes.Size()

	mtr.tx_drop_multicast_packets = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_drop_multicast_packets.Size()

	mtr.tx_drop_multicast_bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_drop_multicast_bytes.Size()

	mtr.tx_drop_broadcast_packets = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_drop_broadcast_packets.Size()

	mtr.tx_drop_broadcast_bytes = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_drop_broadcast_bytes.Size()

	mtr.tx_dma_error = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_dma_error.Size()

	mtr.__rsvd__16 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__16.Size()

	mtr.tx_queue_disabled = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_queue_disabled.Size()

	mtr.tx_queue_sched = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_queue_sched.Size()

	mtr.tx_desc_error = mtr.metrics.GetCounter(offset)
	offset += mtr.tx_desc_error.Size()

	mtr.__rsvd__17 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__17.Size()

	mtr.__rsvd__18 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__18.Size()

	mtr.__rsvd__19 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__19.Size()

	mtr.__rsvd__20 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__20.Size()

	mtr.__rsvd__21 = mtr.metrics.GetCounter(offset)
	offset += mtr.__rsvd__21.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *LifMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "rx_unicast_packets" {
		return offset
	}
	offset += mtr.rx_unicast_packets.Size()

	if fldName == "rx_multicast_packets" {
		return offset
	}
	offset += mtr.rx_multicast_packets.Size()

	if fldName == "rx_broadcast_packets" {
		return offset
	}
	offset += mtr.rx_broadcast_packets.Size()

	if fldName == "__rsvd__0" {
		return offset
	}
	offset += mtr.__rsvd__0.Size()

	if fldName == "rx_unicast_bytes" {
		return offset
	}
	offset += mtr.rx_unicast_bytes.Size()

	if fldName == "rx_broadcast_bytes" {
		return offset
	}
	offset += mtr.rx_broadcast_bytes.Size()

	if fldName == "rx_multicast_bytes" {
		return offset
	}
	offset += mtr.rx_multicast_bytes.Size()

	if fldName == "__rsvd__1" {
		return offset
	}
	offset += mtr.__rsvd__1.Size()

	if fldName == "rx_queue_empty_drops" {
		return offset
	}
	offset += mtr.rx_queue_empty_drops.Size()

	if fldName == "rx_queue_disabled_drops" {
		return offset
	}
	offset += mtr.rx_queue_disabled_drops.Size()

	if fldName == "rx_desc_error" {
		return offset
	}
	offset += mtr.rx_desc_error.Size()

	if fldName == "__rsvd__2" {
		return offset
	}
	offset += mtr.__rsvd__2.Size()

	if fldName == "__rsvd__3" {
		return offset
	}
	offset += mtr.__rsvd__3.Size()

	if fldName == "__rsvd__4" {
		return offset
	}
	offset += mtr.__rsvd__4.Size()

	if fldName == "__rsvd__5" {
		return offset
	}
	offset += mtr.__rsvd__5.Size()

	if fldName == "__rsvd__6" {
		return offset
	}
	offset += mtr.__rsvd__6.Size()

	if fldName == "tx_csum_none" {
		return offset
	}
	offset += mtr.tx_csum_none.Size()

	if fldName == "tx_csum_hw" {
		return offset
	}
	offset += mtr.tx_csum_hw.Size()

	if fldName == "tx_csum_hw_inner" {
		return offset
	}
	offset += mtr.tx_csum_hw_inner.Size()

	if fldName == "tx_csum_partial" {
		return offset
	}
	offset += mtr.tx_csum_partial.Size()

	if fldName == "tx_sg" {
		return offset
	}
	offset += mtr.tx_sg.Size()

	if fldName == "tx_tso" {
		return offset
	}
	offset += mtr.tx_tso.Size()

	if fldName == "tx_tso_sop" {
		return offset
	}
	offset += mtr.tx_tso_sop.Size()

	if fldName == "tx_tso_eop" {
		return offset
	}
	offset += mtr.tx_tso_eop.Size()

	if fldName == "tx_op_error" {
		return offset
	}
	offset += mtr.tx_op_error.Size()

	if fldName == "__rsvd__7" {
		return offset
	}
	offset += mtr.__rsvd__7.Size()

	if fldName == "__rsvd__8" {
		return offset
	}
	offset += mtr.__rsvd__8.Size()

	if fldName == "__rsvd__9" {
		return offset
	}
	offset += mtr.__rsvd__9.Size()

	if fldName == "__rsvd__10" {
		return offset
	}
	offset += mtr.__rsvd__10.Size()

	if fldName == "__rsvd__11" {
		return offset
	}
	offset += mtr.__rsvd__11.Size()

	if fldName == "__rsvd__12" {
		return offset
	}
	offset += mtr.__rsvd__12.Size()

	if fldName == "__rsvd__13" {
		return offset
	}
	offset += mtr.__rsvd__13.Size()

	if fldName == "tx_unicast_packets" {
		return offset
	}
	offset += mtr.tx_unicast_packets.Size()

	if fldName == "tx_unicast_bytes" {
		return offset
	}
	offset += mtr.tx_unicast_bytes.Size()

	if fldName == "tx_multicast_packets" {
		return offset
	}
	offset += mtr.tx_multicast_packets.Size()

	if fldName == "tx_multicast_bytes" {
		return offset
	}
	offset += mtr.tx_multicast_bytes.Size()

	if fldName == "tx_broadcast_packets" {
		return offset
	}
	offset += mtr.tx_broadcast_packets.Size()

	if fldName == "tx_broadcast_bytes" {
		return offset
	}
	offset += mtr.tx_broadcast_bytes.Size()

	if fldName == "__rsvd__14" {
		return offset
	}
	offset += mtr.__rsvd__14.Size()

	if fldName == "__rsvd__15" {
		return offset
	}
	offset += mtr.__rsvd__15.Size()

	if fldName == "tx_drop_unicast_packets" {
		return offset
	}
	offset += mtr.tx_drop_unicast_packets.Size()

	if fldName == "tx_drop_unicast_bytes" {
		return offset
	}
	offset += mtr.tx_drop_unicast_bytes.Size()

	if fldName == "tx_drop_multicast_packets" {
		return offset
	}
	offset += mtr.tx_drop_multicast_packets.Size()

	if fldName == "tx_drop_multicast_bytes" {
		return offset
	}
	offset += mtr.tx_drop_multicast_bytes.Size()

	if fldName == "tx_drop_broadcast_packets" {
		return offset
	}
	offset += mtr.tx_drop_broadcast_packets.Size()

	if fldName == "tx_drop_broadcast_bytes" {
		return offset
	}
	offset += mtr.tx_drop_broadcast_bytes.Size()

	if fldName == "tx_dma_error" {
		return offset
	}
	offset += mtr.tx_dma_error.Size()

	if fldName == "__rsvd__16" {
		return offset
	}
	offset += mtr.__rsvd__16.Size()

	if fldName == "tx_queue_disabled" {
		return offset
	}
	offset += mtr.tx_queue_disabled.Size()

	if fldName == "tx_queue_sched" {
		return offset
	}
	offset += mtr.tx_queue_sched.Size()

	if fldName == "tx_desc_error" {
		return offset
	}
	offset += mtr.tx_desc_error.Size()

	if fldName == "__rsvd__17" {
		return offset
	}
	offset += mtr.__rsvd__17.Size()

	if fldName == "__rsvd__18" {
		return offset
	}
	offset += mtr.__rsvd__18.Size()

	if fldName == "__rsvd__19" {
		return offset
	}
	offset += mtr.__rsvd__19.Size()

	if fldName == "__rsvd__20" {
		return offset
	}
	offset += mtr.__rsvd__20.Size()

	if fldName == "__rsvd__21" {
		return offset
	}
	offset += mtr.__rsvd__21.Size()

	return offset
}

// Setrx_unicast_packets sets cunter in shared memory
func (mtr *LifMetrics) Setrx_unicast_packets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("rx_unicast_packets"))
	return nil
}

// Setrx_multicast_packets sets cunter in shared memory
func (mtr *LifMetrics) Setrx_multicast_packets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("rx_multicast_packets"))
	return nil
}

// Setrx_broadcast_packets sets cunter in shared memory
func (mtr *LifMetrics) Setrx_broadcast_packets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("rx_broadcast_packets"))
	return nil
}

// Set__rsvd__0 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__0"))
	return nil
}

// Setrx_unicast_bytes sets cunter in shared memory
func (mtr *LifMetrics) Setrx_unicast_bytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("rx_unicast_bytes"))
	return nil
}

// Setrx_broadcast_bytes sets cunter in shared memory
func (mtr *LifMetrics) Setrx_broadcast_bytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("rx_broadcast_bytes"))
	return nil
}

// Setrx_multicast_bytes sets cunter in shared memory
func (mtr *LifMetrics) Setrx_multicast_bytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("rx_multicast_bytes"))
	return nil
}

// Set__rsvd__1 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__1"))
	return nil
}

// Setrx_queue_empty_drops sets cunter in shared memory
func (mtr *LifMetrics) Setrx_queue_empty_drops(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("rx_queue_empty_drops"))
	return nil
}

// Setrx_queue_disabled_drops sets cunter in shared memory
func (mtr *LifMetrics) Setrx_queue_disabled_drops(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("rx_queue_disabled_drops"))
	return nil
}

// Setrx_desc_error sets cunter in shared memory
func (mtr *LifMetrics) Setrx_desc_error(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("rx_desc_error"))
	return nil
}

// Set__rsvd__2 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__2"))
	return nil
}

// Set__rsvd__3 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__3"))
	return nil
}

// Set__rsvd__4 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__4"))
	return nil
}

// Set__rsvd__5 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__5(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__5"))
	return nil
}

// Set__rsvd__6 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__6(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__6"))
	return nil
}

// Settx_csum_none sets cunter in shared memory
func (mtr *LifMetrics) Settx_csum_none(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_csum_none"))
	return nil
}

// Settx_csum_hw sets cunter in shared memory
func (mtr *LifMetrics) Settx_csum_hw(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_csum_hw"))
	return nil
}

// Settx_csum_hw_inner sets cunter in shared memory
func (mtr *LifMetrics) Settx_csum_hw_inner(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_csum_hw_inner"))
	return nil
}

// Settx_csum_partial sets cunter in shared memory
func (mtr *LifMetrics) Settx_csum_partial(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_csum_partial"))
	return nil
}

// Settx_sg sets cunter in shared memory
func (mtr *LifMetrics) Settx_sg(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_sg"))
	return nil
}

// Settx_tso sets cunter in shared memory
func (mtr *LifMetrics) Settx_tso(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_tso"))
	return nil
}

// Settx_tso_sop sets cunter in shared memory
func (mtr *LifMetrics) Settx_tso_sop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_tso_sop"))
	return nil
}

// Settx_tso_eop sets cunter in shared memory
func (mtr *LifMetrics) Settx_tso_eop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_tso_eop"))
	return nil
}

// Settx_op_error sets cunter in shared memory
func (mtr *LifMetrics) Settx_op_error(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_op_error"))
	return nil
}

// Set__rsvd__7 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__7(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__7"))
	return nil
}

// Set__rsvd__8 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__8(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__8"))
	return nil
}

// Set__rsvd__9 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__9(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__9"))
	return nil
}

// Set__rsvd__10 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__10(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__10"))
	return nil
}

// Set__rsvd__11 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__11(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__11"))
	return nil
}

// Set__rsvd__12 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__12(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__12"))
	return nil
}

// Set__rsvd__13 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__13(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__13"))
	return nil
}

// Settx_unicast_packets sets cunter in shared memory
func (mtr *LifMetrics) Settx_unicast_packets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_unicast_packets"))
	return nil
}

// Settx_unicast_bytes sets cunter in shared memory
func (mtr *LifMetrics) Settx_unicast_bytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_unicast_bytes"))
	return nil
}

// Settx_multicast_packets sets cunter in shared memory
func (mtr *LifMetrics) Settx_multicast_packets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_multicast_packets"))
	return nil
}

// Settx_multicast_bytes sets cunter in shared memory
func (mtr *LifMetrics) Settx_multicast_bytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_multicast_bytes"))
	return nil
}

// Settx_broadcast_packets sets cunter in shared memory
func (mtr *LifMetrics) Settx_broadcast_packets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_broadcast_packets"))
	return nil
}

// Settx_broadcast_bytes sets cunter in shared memory
func (mtr *LifMetrics) Settx_broadcast_bytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_broadcast_bytes"))
	return nil
}

// Set__rsvd__14 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__14(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__14"))
	return nil
}

// Set__rsvd__15 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__15(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__15"))
	return nil
}

// Settx_drop_unicast_packets sets cunter in shared memory
func (mtr *LifMetrics) Settx_drop_unicast_packets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_drop_unicast_packets"))
	return nil
}

// Settx_drop_unicast_bytes sets cunter in shared memory
func (mtr *LifMetrics) Settx_drop_unicast_bytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_drop_unicast_bytes"))
	return nil
}

// Settx_drop_multicast_packets sets cunter in shared memory
func (mtr *LifMetrics) Settx_drop_multicast_packets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_drop_multicast_packets"))
	return nil
}

// Settx_drop_multicast_bytes sets cunter in shared memory
func (mtr *LifMetrics) Settx_drop_multicast_bytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_drop_multicast_bytes"))
	return nil
}

// Settx_drop_broadcast_packets sets cunter in shared memory
func (mtr *LifMetrics) Settx_drop_broadcast_packets(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_drop_broadcast_packets"))
	return nil
}

// Settx_drop_broadcast_bytes sets cunter in shared memory
func (mtr *LifMetrics) Settx_drop_broadcast_bytes(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_drop_broadcast_bytes"))
	return nil
}

// Settx_dma_error sets cunter in shared memory
func (mtr *LifMetrics) Settx_dma_error(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_dma_error"))
	return nil
}

// Set__rsvd__16 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__16(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__16"))
	return nil
}

// Settx_queue_disabled sets cunter in shared memory
func (mtr *LifMetrics) Settx_queue_disabled(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_queue_disabled"))
	return nil
}

// Settx_queue_sched sets cunter in shared memory
func (mtr *LifMetrics) Settx_queue_sched(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_queue_sched"))
	return nil
}

// Settx_desc_error sets cunter in shared memory
func (mtr *LifMetrics) Settx_desc_error(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("tx_desc_error"))
	return nil
}

// Set__rsvd__17 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__17(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__17"))
	return nil
}

// Set__rsvd__18 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__18(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__18"))
	return nil
}

// Set__rsvd__19 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__19(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__19"))
	return nil
}

// Set__rsvd__20 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__20(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__20"))
	return nil
}

// Set__rsvd__21 sets cunter in shared memory
func (mtr *LifMetrics) Set__rsvd__21(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("__rsvd__21"))
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
