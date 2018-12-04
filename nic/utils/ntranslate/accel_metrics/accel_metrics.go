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
	if lifID, ok := key.(string); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%s", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *accelPfTranslatorFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key string
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type accelSeqQueueTranslatorFns struct{}

// KeyToMeta converts key to meta
func (n *accelSeqQueueTranslatorFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if accel, ok := key.(goproto.AccelSeqQueueKey); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("LifId:%s,QId:%s", accel.LifId, accel.QId)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *accelSeqQueueTranslatorFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key goproto.AccelSeqQueueKey
	fmt.Sscanf(meta.Name, "LifId:%s,QId:%s", &key.LifId, &key.QId)
	return &key
}

type accelHwRingTranslatorFns struct{}

// KeyToMeta converts key to meta
func (n *accelHwRingTranslatorFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if ring, ok := key.(goproto.AccelHwRingKey); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("RId:%s,SubRId:%s", ring.RId, ring.SubRId)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *accelHwRingTranslatorFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key goproto.AccelHwRingKey
	fmt.Sscanf(meta.Name, "RId:%s,SubRId:%s", &key.RId, &key.SubRId)
	return &key
}
