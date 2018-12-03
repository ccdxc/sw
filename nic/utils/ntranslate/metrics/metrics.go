package metrics

import (
	"strconv"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/ntranslate"
)

func init() {
	tstr := ntranslate.MustGetTranslator()

	tstr.Register("LifMetricsKey", &metricsTranslatorFns{})
}

type metricsTranslatorFns struct{}

// KeyToMeta converts network key to meta
func (n *metricsTranslatorFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if str, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: strconv.FormatUint(str, 10)}
	}
	return nil
}

// MetaToKey converts meta to network key
func (n *metricsTranslatorFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	return meta.Name
}
