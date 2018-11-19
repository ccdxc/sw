package accel_metrics

import (
	"github.com/pensando/sw/api"
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
	if lif_id, ok := key.(uint); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("Accel.lif_id..%d", lif_id)}
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
	if accel, ok := key.(AccelSeqQueueKey); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("AccelSeqQueue.lif_id.%d:qid.%d", accel.lif_id, accel.qid)}
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
	if ring, ok := key.(AccelHwRingKey); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("AccelHwRing.rid.%d:sub_rid.%d", ring.rid, ring.sub_rid)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *accelHwRingTranslatorFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	return meta.Name
}
