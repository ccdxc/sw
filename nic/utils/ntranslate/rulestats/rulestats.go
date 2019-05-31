package rulestats

import (
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/ntranslate"
)

func init() {
	tstr := ntranslate.MustGetTranslator()

	tstr.Register("RuleMetricsKey", &ruleTranslatorFns{})
}

type ruleTranslatorFns struct{}

// KeyToMeta converts network key to meta
func (n *ruleTranslatorFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if val, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: fmt.Sprintf("%d", val)}
	}
	return nil
}

// MetaToKey converts meta to network key
func (n *ruleTranslatorFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	return meta.Name
}
