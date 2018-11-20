package accelmetrics

import (
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/proto/goproto"
	"github.com/pensando/sw/venice/utils/ntranslate"
)

func init() {
	tstr := ntranslate.MustGetTranslator()

	tstr.Register("AccelPfInfoKey", &accelPfTranslatorFns{})
	tstr.Register("AccelSeqQueueInfoKey", &accelSeqQueueTranslatorFns{})
	tstr.Register("AccelSeqQueueMetricsKey", &accelSeqQueueTranslatorFns{})
	tstr.Register("AccelHwRingInfoKey", &accelHwRingTranslatorFns{})
	tstr.Register("AccelHwRingMetricsKey", &accelHwRingTranslatorFns{})
}

type accelPfTranslatorFns struct{}

// KeyToMeta converts key to meta
func (n *accelPfTranslatorFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lif_id, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("Accel.LifId.%d", lif_id)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *accelPfTranslatorFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	return meta.Name
}

type accelSeqQueueTranslatorFns struct{}

// KeyToMeta converts key to meta
func (n *accelSeqQueueTranslatorFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if accel, ok := key.(goproto.AccelSeqQueueKey); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("AccelSeqQueue.LifId.%d:QId.%d", accel.LifId, accel.QId)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *accelSeqQueueTranslatorFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	return meta.Name
}

type accelHwRingTranslatorFns struct{}

// KeyToMeta converts key to meta
func (n *accelHwRingTranslatorFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if ring, ok := key.(goproto.AccelHwRingKey); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("AccelHwRing.RId.%d:SubRId.%d", ring.RId, ring.SubRId)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *accelHwRingTranslatorFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	return meta.Name
}
