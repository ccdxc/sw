package upgrade

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/ntranslate"
)

func init() {
	tstr := ntranslate.MustGetTranslator()

	tstr.Register("UpgradeMetricsKey", &upgradeTranslatorFns{})
}

type upgradeTranslatorFns struct{}

// KeyToMeta converts network key to meta
func (n *upgradeTranslatorFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if str, ok := key.(string); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: str}
	}
	return nil
}

// MetaToKey converts meta to network key
func (n *upgradeTranslatorFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	return meta.Name
}
