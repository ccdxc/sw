package sysmond

import (
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/ntranslate"
)

func init() {
	tstr := ntranslate.MustGetTranslator()

	tstr.Register("AsicTemperatureMetricsKey", &asicTemperatureFns{})
	tstr.Register("AsicPowerMetricsKey", &asicPowerFns{})
}

type asicTemperatureFns struct{}

// KeyToMeta converts key to meta
func (n *asicTemperatureFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *asicTemperatureFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}

type asicPowerFns struct{}

// KeyToMeta converts key to meta
func (n *asicPowerFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", lifID)}
	}
	return nil
}

// MetaToKey converts meta to key
func (n *asicPowerFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	var key uint64
	fmt.Sscanf(meta.Name, "%s", &key)
	return &key
}
