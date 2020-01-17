package metrics

import (
	"strconv"

	"github.com/pensando/sw/api"
	clientApi "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/venice/utils/ntranslate"
)

func init() {
	tstr := ntranslate.MustGetTranslator()
	macMetricsXlator := newMacMetricsTranslator()
	tstr.Register("LifMetricsKey", &lifMetricsXlate{})
	tstr.Register("MacMetricsKey", macMetricsXlator)
}

// delphi client
var delphiClient clientApi.Client

type metricsTranslatorFns struct{}

// KeyToMeta converts network key to meta
func (n *metricsTranslatorFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if str, ok := key.(uint64); ok {
		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: "lif" + strconv.FormatUint(str, 10)}
	}
	return nil
}

// MetaToKey converts meta to network key
func (n *metricsTranslatorFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	return meta.Name
}

// SetDelphiClient sets the delphi client to be used for ntranslate
func SetDelphiClient(dclient clientApi.Client) {
	delphiClient = dclient
}
