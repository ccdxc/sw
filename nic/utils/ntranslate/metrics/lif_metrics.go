package metrics

import (
	"strconv"

	"github.com/pensando/sw/api"
	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	dnetproto "github.com/pensando/sw/nic/agent/protos/generated/delphi/netproto/delphi"
)

type lifMetricsXlate struct{}

// KeyToMeta converts network key to meta
func (n *lifMetricsXlate) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		intfName := strconv.FormatUint(uint64(lifID), 10)
		if delphiClient != nil {
			nodeUUID := ""
			nslist := delphiProto.DistributedServiceCardStatusList(delphiClient)
			for _, ns := range nslist {
				nodeUUID = ns.GetDSCName()
			}
			intfList := dnetproto.InterfaceList(delphiClient)
			for _, intf := range intfList {
				if intf.Interface.Status.InterfaceID == lifID {
					if nodeUUID != "" {
						intfName = nodeUUID + "-" + intf.Interface.ObjectMeta.Name
					} else {
						intfName = intf.Interface.ObjectMeta.Name
					}
				}
			}
		}

		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: intfName}
	}
	return nil
}

// MetaToKey converts meta to network key
func (n *lifMetricsXlate) MetaToKey(meta *api.ObjectMeta) interface{} {
	return meta.Name
}
