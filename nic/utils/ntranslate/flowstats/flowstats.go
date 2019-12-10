package flowstats

import (
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/proto/goproto"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/ntranslate"
)

func init() {
	tstr := ntranslate.MustGetTranslator()

	tstr.Register("IPv4FlowDropMetricsKey", &v4FlowDropFns{})
}

type v4FlowDropFns struct{}

// GetFlowMeta converts Key to ObjectMeta for flowstats
func GetFlowMeta(val goproto.IPv4FlowKey) *api.ObjectMeta {
	ipSrc := netutils.IPv4Uint32ToString(val.Sip)
	ipDest := netutils.IPv4Uint32ToString(val.Dip)
	dPort := fmt.Sprintf("%v", val.Dport)
	sPort := fmt.Sprintf("%v", val.Sport)
	ipProt := fmt.Sprintf("%v", val.Ip_proto)

	return &api.ObjectMeta{Tenant: "default",
		Namespace: "default",
		Name:      "flowstats",
		Labels: map[string]string{
			"destination-port": dPort,
			"destination":      ipDest,
			"source":           ipSrc,
			"source-port":      sPort,
			"protocol":         ipProt,
		},
	}

}

// KeyToMeta converts network key to meta
func (n *v4FlowDropFns) KeyToMeta(key interface{}) *api.ObjectMeta {
	if val, ok := key.(goproto.IPv4FlowKey); ok {
		return GetFlowMeta(val)
	}
	return nil
}

// MetaToKey converts meta to network key
func (n *v4FlowDropFns) MetaToKey(meta *api.ObjectMeta) interface{} {
	return meta.Name
}
